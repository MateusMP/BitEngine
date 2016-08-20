#pragma once

#include <glm\common.hpp>

#include "Core/VideoRenderer.h"
#include "Core/ECS/EntitySystem.h"
#include "Core/SpriteManager.h"

#include "Core/Graphics/Shader.h"
#include "Core/ECS/Transform2DProcessor.h"
#include "Core/ECS/Camera2DComponent.h"

namespace BitEngine
{

	class Sprite2DComponent2 : public Component<Sprite2DComponent2>
	{
		public:
			s32 layer;
			Sprite* sprite;
			float alpha;
	};

	// Doesn't use instanced rendering
	struct Sprite2D_DD_legacy
	{
		struct VertexContainer {
			glm::vec2 position;
			glm::vec2 textureUV;
		};

		struct CamMatricesContainer {
			glm::mat4 view;
		};

		struct TextureContainer {
			const Texture* diffuse;
		};

		void init(Shader* shader)
		{
			m_vertexContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 0);

			u_viewMatrixContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 0);
			m_textureContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 1);
		}

		ShaderDataReference m_vertexContainer;
		ShaderDataReference u_viewMatrixContainer;
		ShaderDataReference m_textureContainer;
	};

	// Instanced rendering
	struct Sprite2D_DD_new
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
			const Texture* diffuse;
		};

		void init(Shader* shader)
		{
			m_ptnContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 0);
			u_modelMatrixContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 1);

			u_viewMatrixContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 0);
			m_textureContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 1);
		}

		ShaderDataReference m_ptnContainer;
		ShaderDataReference u_modelMatrixContainer;
		ShaderDataReference u_viewMatrixContainer;
		ShaderDataReference m_textureContainer;
	};

	class Sprite2DRenderer : public ComponentProcessor
	{
		public: 
			Sprite2DRenderer(GameEngine* engine);

			bool Init() override;
			void Stop() override;

			void setActiveCamera(ComponentRef<Camera2DComponent>& handle);
			void Render();

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

			typedef std::pair<u32, const Texture*> BatchIdentifier;

			class Sprite2DBatch
			{
				friend class Sprite2DRenderer;
				public:
					BatchIdentifier bid;
					std::vector<SpriteBatchInstance> batchInstances;
			};

			void prepare_legacy(Sprite2DBatch& batch);
            void buildLegacySpriteVertices(std::vector<SpriteBatchInstance>& batchInstances, Sprite2D_DD_legacy::VertexContainer* vertexContainer, const u32 vtxIdx, const u32 idx);

			void prepare_new(Sprite2DBatch& batch);

			void buildBatchInstances();

			std::map<BatchIdentifier, u32> batchesMap;
			std::vector<Sprite2DBatch> batches;
			ComponentRef<Camera2DComponent> activeCamera;
			IGraphicBatch* m_batch;
			GameEngine* m_engine;
			Shader* shader;
			Sprite2D_DD_legacy legacyRefs;
			Sprite2D_DD_new newRefs;
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