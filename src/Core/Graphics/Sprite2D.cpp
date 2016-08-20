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
		shader = m_engine->getResourceLoader()->getResource<Shader>("data/shaders/sprite2D");
		if (shader == nullptr) {
			return false;
		}

		legacyRefs.init(shader);
		newRefs.init(shader);
		m_batch = shader->createBatch();

		return true;
	}

	void Sprite2DRenderer::Stop()
	{
		delete m_batch;
	}

	void Sprite2DRenderer::Render()
	{
		if (m_batch == nullptr)
		{
			if (!shader->isReady()) {
				return;
			} else {
				m_batch = shader->createBatch();
			}
		}

		m_batch->clear();

		buildBatchInstances();

		if (getES()->getEngine()->getVideoDriver()->getVideoAdapter() == VideoAdapterType::OPENGL_4)
		{
			for (auto& it : batchesMap) {
				prepare_new(batches[it.second]);
				m_batch->load();
				m_batch->render(shader);
			}
		}
		else
		{
			for (auto& it : batchesMap) {
				prepare_legacy(batches[it.second]);
				m_batch->load();
				m_batch->render(shader);
			}
		}
	}

	void Sprite2DRenderer::buildBatchInstances()
	{
		for (Sprite2DBatch& it : batches) {
			it.batchInstances.clear();
		}

		// Build batch
		getES()->forEach<SceneTransform2DComponent, Sprite2DComponent2>(
			[this](const ComponentRef<SceneTransform2DComponent>& transform, const ComponentRef<Sprite2DComponent2>& sprite)
		{
			BatchIdentifier idtf(sprite->layer, sprite->sprite->getTexture());
			const auto& it = batchesMap.find(idtf);
			if (it != batchesMap.end()) {
				batches[it->second].batchInstances.emplace_back(transform.ref(), sprite.ref());
			} else {
				batches.emplace_back();
				u32 id = batches.size() - 1;
				batchesMap.emplace(idtf, id);
				batches[id].bid = idtf;
				batches[id].batchInstances.emplace_back(transform.ref(), sprite.ref());
			}
		});
	}

    void Sprite2DRenderer::buildLegacySpriteVertices(std::vector<SpriteBatchInstance>& batchInstances, Sprite2D_DD_legacy::VertexContainer* vertexContainer, const u32 vtxOffset, const u32 idx){
        const SpriteBatchInstance& inst = batchInstances[idx];
        const Sprite* lastSprite = inst.sprite.sprite;
        const glm::vec4& uvrect = lastSprite->getUV();
        const glm::vec2 sizes(lastSprite->getWidth(), lastSprite->getHeight());
        const glm::vec2 offsets(-lastSprite->getOffsetX(), -lastSprite->getOffsetY());
        const glm::vec2 off_siz = offsets*sizes;
        const glm::vec2 vertex_pos[4] = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f)
        };
        vertexContainer[vtxOffset + 0].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[0] * sizes + off_siz, 1));
        vertexContainer[vtxOffset + 1].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
        vertexContainer[vtxOffset + 2].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
        vertexContainer[vtxOffset + 3].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
        vertexContainer[vtxOffset + 4].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
        vertexContainer[vtxOffset + 5].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[3] * sizes + off_siz, 1));
        
        vertexContainer[vtxOffset + 0].textureUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw glm::vec2(0, 1);
        vertexContainer[vtxOffset + 1].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR  		 glm::vec2(0, 0);
        vertexContainer[vtxOffset + 2].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL  		 glm::vec2(1, 0);
        vertexContainer[vtxOffset + 3].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL  		 glm::vec2(0, 1);
        vertexContainer[vtxOffset + 4].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR  		 glm::vec2(0, 0);
        vertexContainer[vtxOffset + 5].textureUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy glm::vec2(1, 1);
    }

	void Sprite2DRenderer::prepare_legacy(Sprite2DBatch& batch)
	{
		std::vector<SpriteBatchInstance>& batchInstances = batch.batchInstances;
		m_batch->clear();
		Sprite2D_DD_legacy::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_legacy::TextureContainer>(legacyRefs.m_textureContainer);
		texture->diffuse = batch.bid.second;

		const int N_VERTEX_PER_QUAD = 6;
		m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLES);

		// Prepare for all instances
		u32 nInstances = batchInstances.size();
		u32 nVertices = nInstances * N_VERTEX_PER_QUAD;
		m_batch->prepare(nVertices); // quad sprites need 4 vertices for each

		// Setup batch data
		if (!batchInstances.empty())
		{
			Sprite2D_DD_legacy::VertexContainer* vertexContainer = m_batch->getShaderDataAs<Sprite2D_DD_legacy::VertexContainer>(legacyRefs.m_vertexContainer);

			//const u32 instanceCount = batchInstances.size();
			for (u32 idx = 0; idx < nInstances; ++idx)
			{
				buildLegacySpriteVertices(batchInstances, vertexContainer, idx * N_VERTEX_PER_QUAD, idx);
			}
		}
		Sprite2D_DD_legacy::CamMatricesContainer* view = m_batch->getShaderDataAs<Sprite2D_DD_legacy::CamMatricesContainer>(legacyRefs.u_viewMatrixContainer);
		if (view) {
			view->view = activeCamera->getMatrix();
		}
	}

	void Sprite2DRenderer::prepare_new(Sprite2DBatch& batch)
	{
		std::vector<SpriteBatchInstance>& batchInstances = batch.batchInstances;

		m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
		// Prepare for all instances
		m_batch->prepare(batchInstances.size());

		// Setup batch data
		if (!batchInstances.empty())
		{
			Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(newRefs.m_textureContainer);
			texture->diffuse = batch.bid.second;

			Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(newRefs.u_modelMatrixContainer);
			Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(newRefs.m_ptnContainer);

			const u32 instanceCount = batchInstances.size();
			for (u32 i = 0; i < instanceCount; ++i)
			{
				const SpriteBatchInstance& inst = batchInstances[i];

				modelMatrices[i].modelMatrix = inst.transform.m_global;

				//vertices[i].position = ? ;
				vertices[i].textureUV = inst.sprite.sprite->getUV();
			}
		}
	}
}