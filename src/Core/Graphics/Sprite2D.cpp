#include "Core/Graphics/Sprite2D.h"

#include "Core/VideoSystem.h"

namespace BitEngine {

	Sprite2DRenderer::Sprite2DRenderer(GameEngine* engine)
		: ComponentProcessor(engine->getMessenger()), m_engine(engine)
	{
	}

	void Sprite2DRenderer::setActiveCamera(ComponentRef<Camera2DComponent>& handle)
	{
		activeCamera = handle;
	}

	bool Sprite2DRenderer::Init()
	{
		Shader* shader = m_engine->getResourceLoader()->getResource<Shader>("data/shaders/sprite2D");
		if (shader == nullptr)
		{
			return false;
		}

		m_shader = new Sprite2DShaderWrapper(shader);
		m_batch = shader->createBatch();

		return true;
	}

	void Sprite2DRenderer::Stop()
	{
		delete m_shader;
		delete m_batch;
	}

	void Sprite2DRenderer::Prepare()
	{
		if (m_batch == nullptr)
		{
			if (!m_shader->getShader()->isReady()) {
				return;
			} else {
				m_batch = m_shader->getShader()->createBatch();
			}
		}

		m_batch->clear();

		buildBatchInstances();

		if (getES()->getEngine()->getVideoDriver()->getVideoAdapter() == VideoAdapterType::OPENGL_4)
		{
			prepare_new();
		}
		else
		{
			prepare_legacy();
		}

		// Make sure it's on gpu
		m_batch->load();
	}

	void Sprite2DRenderer::buildBatchInstances()
	{
		batchInstances.clear();
		// Build batch
		getES()->forEach<SceneTransform2DComponent, Sprite2DComponent2>(
			[this](const ComponentRef<SceneTransform2DComponent>& transform, const ComponentRef<Sprite2DComponent2>& sprite)
		{
			batchInstances.emplace_back(transform.ref(), sprite.ref());
		});

		// Sort batch
		std::sort(batchInstances.begin(), batchInstances.end(), [](const SpriteBatchInstance& a, const SpriteBatchInstance& b) {
			return (a.sprite.layer < b.sprite.layer) && (a.sprite.sprite < b.sprite.sprite);
		});
	}

	void Sprite2DRenderer::prepare_legacy()
	{
		const int N_VERTEX_PER_QUAD = 6;
		m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLES);

		// Prepare for all instances
		u32 nInstances = batchInstances.size();
		u32 nVertices = nInstances * N_VERTEX_PER_QUAD;
		m_batch->prepare(nVertices); // quad sprites need 4 vertices for each

		// DEBUG RENDER TEST
		m_shader->getShader()->Bind();

		// Setup batch data
		if (!batchInstances.empty())
		{
			IBatchSector* currentSector = nullptr;
			Sprite2D_DD_legacy::TextureContainer* texture = nullptr;
			const ITexture* lastSpriteTexture = nullptr;
			Sprite* lastSprite = nullptr;

			auto* vertexContainer = m_batch->getShaderDataAs<Sprite2D_DD_legacy::VertexContainer>(m_shader->getVertexContainerRef());

			//const u32 instanceCount = batchInstances.size();
			for (u32 idx = 0; idx < nInstances; ++idx)
			{
				const SpriteBatchInstance& inst = batchInstances[idx];
				const u32 vtxOffset = idx * N_VERTEX_PER_QUAD;

				// Handle batch sectors
				if (inst.sprite.sprite != lastSprite)
				{
					const ITexture* curTexture = inst.sprite.sprite->getTexture();
					if (lastSpriteTexture != curTexture) {
						lastSpriteTexture = curTexture;
						if (currentSector != nullptr) {
							currentSector->end(vtxOffset);
						}
						currentSector = m_batch->addSector(vtxOffset);
						lastSprite = inst.sprite.sprite;
						texture = currentSector->getConfigValueAs<Sprite2D_DD_legacy::TextureContainer>(m_shader->getTextureContainerRef());
						if (texture != nullptr)
						{
							texture->diffuse = lastSpriteTexture;
						}
					}
				}
				
				const glm::vec4& uvrect = lastSprite->getUV();
				vertexContainer[vtxOffset + 0].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3(-1, -1, 1)));
				vertexContainer[vtxOffset + 1].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3(-1,  1, 1)));
				vertexContainer[vtxOffset + 2].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3( 1,  1, 1)));
				vertexContainer[vtxOffset + 3].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3(-1, -1, 1)));
				vertexContainer[vtxOffset + 4].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3( 1,  1, 1)));
				vertexContainer[vtxOffset + 5].position = glm::vec2(inst.transform.m_global * (128.0f * glm::vec3( 1, -1, 1)) );

				vertexContainer[vtxOffset + 0].textureUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw glm::vec2(0, 1);
				vertexContainer[vtxOffset + 1].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR  		 glm::vec2(0, 0);
				vertexContainer[vtxOffset + 2].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL  		 glm::vec2(1, 0);
				vertexContainer[vtxOffset + 3].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL  		 glm::vec2(0, 1);
				vertexContainer[vtxOffset + 4].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR  		 glm::vec2(0, 0);
				vertexContainer[vtxOffset + 5].textureUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy glm::vec2(1, 1);
				/*
				glBegin(GL_TRIANGLES);
				for (int k = 0; k < 6; ++k) {
					glVertex3f(vertexContainer[vtxOffset + k].position.x, vertexContainer[vtxOffset + k].position.y, 0);
				}
				glEnd();*/
				
				//LOG(EngineLog, BE_LOG_VERBOSE) << "---\n";
				//LOG(EngineLog, BE_LOG_VERBOSE) << vertex.position.x << "," << vertex.position.y;
			}

			if (currentSector != nullptr) {
				currentSector->end(nVertices);
			}
		}
		Sprite2D_DD_legacy::CamMatricesContainer* view = m_batch->getShaderDataAs<Sprite2D_DD_legacy::CamMatricesContainer>(m_shader->getViewMatrixContainerRef());
		if (view)
		{
			view->view = activeCamera->getMatrix();
		}
		m_batch->load();
		m_shader->getShader()->Unbind();
	}

	void Sprite2DRenderer::prepare_new()
	{
		m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
		// Prepare for all instances
		m_batch->prepare(batchInstances.size());

		// Setup batch data
		if (!batchInstances.empty())
		{
			IBatchSector* currentSector = nullptr;
			Sprite2D_DD_new::TextureContainer* texture = nullptr;
			const ITexture* lastSpriteTexture = nullptr;
			Sprite* lastSprite = nullptr;

			Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(m_shader->getModelMatrixContainerRef());
			Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(m_shader->getModelMatrixContainerRef());

			const u32 instanceCount = batchInstances.size();
			for (u32 i = 0; i < instanceCount; ++i)
			{
				const SpriteBatchInstance& inst = batchInstances[i];

				// Handle batch sectors
				if (inst.sprite.sprite != lastSprite)
				{
					const ITexture* curTexture = inst.sprite.sprite->getTexture();
					if (lastSpriteTexture != curTexture) {
						lastSpriteTexture = curTexture;
						currentSector = m_batch->addSector(i);
						texture = currentSector->getConfigValueAs<Sprite2D_DD_new::TextureContainer>(m_shader->getTextureContainerRef());
						lastSprite = inst.sprite.sprite;
						texture->diffuse = lastSpriteTexture;
					}
				}

				modelMatrices[i].modelMatrix = inst.transform.m_global;

				//vertices[i].position = ? ;
				vertices[i].textureUV = lastSprite->getUV();
			}
		}
	}

	void Sprite2DRenderer::Render()
	{
		if (m_batch != nullptr)
		{
			m_batch->render(m_shader->getShader());
		}
	}

}