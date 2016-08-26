#pragma once

#include <glm\common.hpp>

#include "Core/Graphics/VideoRenderer.h"
#include "Core/ECS/EntitySystem.h"
#include "Core/SpriteManager.h"

#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Material.h"
#include "Core/ECS/Transform2DProcessor.h"
#include "Core/ECS/Camera2DComponent.h"

namespace BitEngine
{
	class Sprite2DComponent : public Component<Sprite2DComponent>
	{
		public:
		Sprite2DComponent(u32 _layer, RR<Sprite> spr, const Material* mat)
			: layer(_layer), alpha(1.0f), sprite(spr), material(mat)
		{
		}

		s32 layer;
		float alpha;
		RR<Sprite> sprite;
		const Material* material;
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
			const static Material* DEFAULT_SPRITE;
			const static Material* TRANSPARENT_SPRITE;
			const static Material* EFFECT_SPRITE;

			Sprite2DRenderer(GameEngine* engine);

			bool Init() override;
			void Stop() override;

			void setActiveCamera(ComponentRef<Camera2DComponent>& handle);
			void Render();

		private:
			struct SpriteBatchInstance {
				SpriteBatchInstance(const SceneTransform2DComponent& t, const Sprite2DComponent& s)
					: transform(t), sprite(s)
				{}

				SpriteBatchInstance& operator= (const SpriteBatchInstance& other) {
					return *(new(this)SpriteBatchInstance(other));
				}

				const SceneTransform2DComponent& transform;
				const Sprite2DComponent& sprite;
			};

			struct BatchIdentifier {
				BatchIdentifier(u32 _layer, const Material* mat, const Texture* tex)
					: layer(_layer), material(mat), texture(tex)
				{}

				bool operator<(const BatchIdentifier& o) const {
					return layer < o.layer || material < o.material || texture < o.texture;
				}

				u32 layer;
				const Material* material;
				const Texture* texture;
			};

			static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, float radius)
			{
				const float kX = matrix[2][0] + radius;
				const float kX_r = matrix[2][0] - radius;
				const float kY = matrix[2][1] + radius;
				const float kY_b = matrix[2][1] - radius;

				if (kX < screen.x) {
					// printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
					return false;
				}
				if (kX_r > screen.z) {
					//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
					return false;
				}
				if (kY < screen.y) {
					//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
					return false;
				}
				if (kY_b > screen.w) {
					//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
					return false;
				}

				return true;
			}

			class Sprite2DBatch
			{
				friend class Sprite2DRenderer;
				public:
					Sprite2DBatch(const BatchIdentifier& bi)
						: bid(bi)
					{}

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
			RR<Shader> shader;
			Sprite2D_DD_legacy legacyRefs;
			Sprite2D_DD_new newRefs;
	};

	template<>
	class ComponentHolder<Sprite2DComponent> : public BaseComponentHolder
	{
		public:
			ComponentHolder(GameEngine* _engine)
				: BaseComponentHolder(_engine->getMessenger(), sizeof(Sprite2DComponent)), engine(_engine)
			{

			}

			bool init() override {
				defaultSprite = engine->getResourceLoader()->getResource<Sprite>("data/default/sprite");
				return defaultSprite.isValid();
			}

			void sendDestroyMessage(EntityHandle entity, ComponentHandle component) override {
				sendComponentDestroyedMessage<Sprite2DComponent>(entity, component);
			}

			template<typename ... Args>
			void initializeComponent(Sprite2DComponent* outPtr, Args ...args) {
				initComponent(outPtr, args...);
			}
			

		private:
			template<typename ... Args>
			void initComponent(Sprite2DComponent* outPtr, Args ...args) {
				new (outPtr) Sprite2DComponent(args...);
			}

			void initComponent(Sprite2DComponent* outPtr) {
				new (outPtr) Sprite2DComponent(0, defaultSprite, Sprite2DRenderer::DEFAULT_SPRITE);
			}


			GameEngine* engine;
			RR<Sprite> defaultSprite;
	};


}
