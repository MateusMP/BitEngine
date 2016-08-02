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

	struct Sprite2DDataDefinition 
	{
		struct PTNContainer {
			glm::vec2 position;
			glm::vec4 textureUV;
		};

		struct ModelMatrixContainer {
			glm::mat3 modelMatrix;
		};

		struct CamMatricesContainer {
			glm::mat4 projection;
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
				m_modelMatrixContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 1);

				m_textureContainer = m_shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 0);
			}
			
			ShaderDataDefinition::DefinitionReference& getTextureContainerRef() {
				return m_textureContainer;
			}

			ShaderDataDefinition::DefinitionReference& getModelMatrixContainerRef() {
				return m_modelMatrixContainer;
			}

			ShaderDataDefinition::DefinitionReference& getPTNContainerRef() {
				return m_ptnContainer;
			}

			Shader* getShader() {
				return m_shader;
			}

		private:
			Shader* m_shader;
			ShaderDataDefinition::DefinitionReference m_textureContainer;
			ShaderDataDefinition::DefinitionReference m_ptnContainer;
			ShaderDataDefinition::DefinitionReference m_modelMatrixContainer;
	};

	class Sprite2DRenderer : public ComponentProcessor
	{
		public: 
		Sprite2DRenderer(GameEngine* engine)
			: ComponentProcessor(engine->getMessenger()), m_engine(engine)
		{
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
			m_batch->clear();
						
			// Build batch
			getES()->forEach<SceneTransform2DComponent, Sprite2DComponent2>(
					[this](const ComponentRef<SceneTransform2DComponent>& transform, const ComponentRef<Sprite2DComponent2>& sprite)
				{
					batchInstances.emplace_back(transform.ref(), sprite.ref());
				}
			);
			
			// Sort batch
			std::sort(batchInstances.begin(), batchInstances.end(), [](const SpriteBatchInstance& a, const SpriteBatchInstance& b) {
				return (a.sprite.layer < b.sprite.layer) && (a.sprite.sprite < b.sprite.sprite);
			});

			// Prepare for all instances
			m_batch->prepare(batchInstances.size());

			// Setup batch data
			if (!batchInstances.empty())
			{
				IBatchSector* currentSector = nullptr;
				Sprite2DDataDefinition::TextureContainer* texture = nullptr;
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
							texture = currentSector->getConfigValueAs<Sprite2DDataDefinition::TextureContainer>(m_shader->getTextureContainerRef());
							lastSprite = inst.sprite.sprite;
							texture->diffuse = lastSpriteTexture;
						}
					}

					m_batch->getVertexDataAddressAs<Sprite2DDataDefinition::ModelMatrixContainer>(m_shader->getModelMatrixContainerRef(), i)->modelMatrix = inst.transform.m_global;

					Sprite2DDataDefinition::PTNContainer* ptn = (m_batch->getVertexDataAddressAs<Sprite2DDataDefinition::PTNContainer>(m_shader->getPTNContainerRef(), i));
					ptn->textureUV = lastSprite->getUV();
				}
			}

			// Make sure it's on gpu
			m_batch->load();
		}

		void Render()
		{
			m_batch->render(m_shader->getShader());
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