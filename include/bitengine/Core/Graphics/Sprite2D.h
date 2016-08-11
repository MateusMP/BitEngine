#pragma once

#include <glm\common.hpp>

#include "Core/VideoRenderer.h"
#include "Core/ECS/EntitySystem.h"
#include "Core/SpriteManager.h"

#include "Core/Graphics/Shader.h"
#include "Core/ECS/Transform2DProcessor.h"

namespace BitEngine
{

	class Sprite2DComponent2 : public Component<Sprite2DComponent2>
	{
		public:
			int layer;
			Sprite* sprite;
			float alpha;
	};

	// Doesn't use instanced rendering
	struct Sprite2DDataDefinition_legacy
	{
		struct VertexContainer {
			glm::vec2 position;
			glm::vec2 textureUV;
		};

		struct CamMatricesContainer {
			glm::mat4 view;
		};

		struct TextureContainer {
			const ITexture* diffuse;
		};
	};

	// Instanced rendering
	struct Sprite2DDataDefinition_new
	{
		struct PTNContainer {
			glm::vec2 position;
			glm::vec4 textureUV;
		};

		struct ModelMatrixContainer {
			glm::mat3 modelMatrix;
		};

		struct CamMatricesContainer {
			glm::mat4 view;
		};

		struct TextureContainer {
			const ITexture* diffuse;
		};
	};

	class Sprite2DShaderWrapper
	{
		public:
			struct SpriteVertex {
				glm::vec4 uvrect;
			};

			Sprite2DShaderWrapper(Shader* shader)
				: m_shader(shader)
			{
				m_ptnContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 0);
				m_vertexContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 0);
				u_modelMatrixContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 1);

				u_viewMatrixContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 0);
				m_textureContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 1);
			}
			
			ShaderDataDefinition::DefinitionReference& getTextureContainerRef() {
				return m_textureContainer;
			}

			ShaderDataDefinition::DefinitionReference& getModelMatrixContainerRef() {
				return u_modelMatrixContainer;
			}

			ShaderDataDefinition::DefinitionReference& getViewMatrixContainerRef() {
				return u_viewMatrixContainer;
			}

			ShaderDataDefinition::DefinitionReference& getPTNContainerRef() {
				return m_ptnContainer;
			}

			ShaderDataDefinition::DefinitionReference& getVertexContainerRef() {
				return m_vertexContainer;
			}

			Shader* getShader() {
				return m_shader;
			}

			bool preMultiplyModelMatrix() {
				return m_shader->getDefinition().checkRef(u_modelMatrixContainer);
			}

		private:
			Shader* m_shader;
			bool preModel;
			ShaderDataDefinition::DefinitionReference m_textureContainer;
			ShaderDataDefinition::DefinitionReference m_vertexContainer;
			ShaderDataDefinition::DefinitionReference m_ptnContainer;
			ShaderDataDefinition::DefinitionReference u_viewMatrixContainer;
			ShaderDataDefinition::DefinitionReference u_modelMatrixContainer;
	};

	class Sprite2DRenderer : public ComponentProcessor
	{
		public: 
		Sprite2DRenderer(GameEngine* engine)
			: ComponentProcessor(engine->getMessenger()), m_engine(engine)
		{
		}

		void setActiveCamera(ComponentRef<Camera2DComponent>& handle)
		{
			activeCamera = handle;
		}

		bool Init() override
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

		void Stop() override
		{
			delete m_shader;
			delete m_batch;
		}

		void Prepare()
		{
			if (m_batch == nullptr)
			{
				if (!m_shader->getShader()->isReady())
				{
					return;
				}
				else
				{
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

		void buildBatchInstances()
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

		void prepare_legacy()
		{
			const int N_VERTEX_PER_QUAD = 6;
			m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLES);
			// Prepare for all instances
			u32 nVertices = batchInstances.size() * N_VERTEX_PER_QUAD;
			m_batch->prepare(nVertices); // quad sprites need 4 vertices for each

			// CW
			const glm::vec3 quad_[] = {
				{-0.5,-0.5, 1},	// 1 | 1  2
				{ 0.5,-0.5, 1},	// 2 | 
				{-0.5, 0.5, 1},	// 3 | 
				{ 0.5,-0.5, 1},	// 2 | 
				{-0.5, 0.5, 1},	// 3 | 
				{ 0.5, 0.5, 1}	// 4 | 3  4
			};

			// CCW
			const glm::vec3 quad[] = {
				{-0.5, 0.5, 1},	// 1 | 1   24
				{ 0.5, 0.5, 1},	// 2 | 
				{-0.5,-0.5, 1},	// 3 | 
				{ 0.5, 0.5, 1},	// 4 | 
				{ 0.5,-0.5, 1},	// 5 | 
				{-0.5,-0.5, 1}	// 6 | 36  45
			};

			const glm::u16vec2 uvs[] = {
				{0, 0},
				{2, 0},
				{0, 3},
				{2, 0},
				{0, 3},
				{2, 3},
			};

			m_shader->getShader()->Bind();

			// Setup batch data
			if (!batchInstances.empty())
			{
				IBatchSector* currentSector = nullptr;
				Sprite2DDataDefinition_legacy::TextureContainer* texture = nullptr;
				const ITexture* lastSpriteTexture = nullptr;
				Sprite* lastSprite = nullptr;
								
				//const u32 instanceCount = batchInstances.size();
				for (u32 i = 0; i < nVertices; ++i)
				{
					const int vIdx = i % N_VERTEX_PER_QUAD;
					const int idx = (i / N_VERTEX_PER_QUAD);
					const SpriteBatchInstance& inst = batchInstances[idx];

					// Handle batch sectors
					if (inst.sprite.sprite != lastSprite)
					{
						const ITexture* curTexture = inst.sprite.sprite->getTexture();
						if (lastSpriteTexture != curTexture) {
							lastSpriteTexture = curTexture;
							if (currentSector != nullptr) {
								currentSector->end(i);
							}
							currentSector = m_batch->addSector(i);
							lastSprite = inst.sprite.sprite;
							texture = currentSector->getConfigValueAs<Sprite2DDataDefinition_legacy::TextureContainer>(m_shader->getTextureContainerRef());
							if (texture != nullptr)
							{
								texture->diffuse = lastSpriteTexture;
							}
						}
					}

					auto* vertexContainer = m_batch->getVertexDataAddressAs<Sprite2DDataDefinition_legacy::VertexContainer>(m_shader->getVertexContainerRef(), i);
					const glm::vec4& uv = lastSprite->getUV();
					vertexContainer->position = glm::vec2((128.0f * quad[vIdx]) * inst.transform.m_global);
					vertexContainer->textureUV = glm::vec2(uv[uvs[vIdx].x], uv[uvs[vIdx].y]);
					
					if (vIdx == 0) {
						glBegin(GL_TRIANGLES);
					}
					glVertex3f(vertexContainer->position.x, vertexContainer->position.y, 1);
					LOG(EngineLog, BE_LOG_VERBOSE) << vertexContainer->position.x << "," << vertexContainer->position.y;
					if (vIdx == 5) {
						glEnd();
						LOG(EngineLog, BE_LOG_VERBOSE) << "---\n";
					}
				}
				if (currentSector != nullptr) {
					currentSector->end(nVertices);
				}
			}
			Sprite2DDataDefinition_legacy::CamMatricesContainer* view = m_batch->getConfigDataAddressAs<Sprite2DDataDefinition_legacy::CamMatricesContainer>(m_shader->getViewMatrixContainerRef());
			view->view = activeCamera->getMatrix();
			m_batch->load();
			m_shader->getShader()->Unbind();
		}

		void prepare_new()
		{
			m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
			// Prepare for all instances
			m_batch->prepare(batchInstances.size());

			// Setup batch data
			if (!batchInstances.empty())
			{
				IBatchSector* currentSector = nullptr;
				Sprite2DDataDefinition_new::TextureContainer* texture = nullptr;
				const ITexture* lastSpriteTexture = nullptr;
				Sprite* lastSprite = nullptr;

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
							texture = currentSector->getConfigValueAs<Sprite2DDataDefinition_new::TextureContainer>(m_shader->getTextureContainerRef());
							lastSprite = inst.sprite.sprite;
							texture->diffuse = lastSpriteTexture;
						}
					}

					m_batch->getVertexDataAddressAs<Sprite2DDataDefinition_new::ModelMatrixContainer>(m_shader->getModelMatrixContainerRef(), i)->modelMatrix = inst.transform.m_global;

					Sprite2DDataDefinition_new::PTNContainer* ptn = (m_batch->getVertexDataAddressAs<Sprite2DDataDefinition_new::PTNContainer>(m_shader->getPTNContainerRef(), i));
					ptn->textureUV = lastSprite->getUV();
				}
			}
		}

		void Render()
		{
			if (m_batch != nullptr)
			{
				m_batch->render(m_shader->getShader());
			}
		}

		private:
			struct SpriteBatchInstance {
				SpriteBatchInstance(const SceneTransform2DComponent& t, const Sprite2DComponent2& s)
					: transform(t), sprite(s)
				{
				}

				SpriteBatchInstance& operator= (const SpriteBatchInstance& other) {
					return *(new(this)SpriteBatchInstance(other));
				}

				const SceneTransform2DComponent& transform;
				const Sprite2DComponent2& sprite;
			};


			std::vector<SpriteBatchInstance> batchInstances;
			ComponentRef<Camera2DComponent> activeCamera;

			GameEngine* m_engine;
			IGraphicBatch* m_batch;
			Sprite2DShaderWrapper* m_shader;
	};

	class Sprite2DHolder : public ComponentHolder<Sprite2DComponent2>
	{
		public:
			Sprite2DHolder(Sprite2DRenderer* renderer)
				: ComponentHolder(renderer->getMessenger())
			{

			}

		private:

	};


}