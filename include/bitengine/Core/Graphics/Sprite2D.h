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
			int layer;
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
			const ITexture* diffuse;
		};
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
			
			ShaderDataReference& getTextureContainerRef() {
				return m_textureContainer;
			}

			ShaderDataReference& getModelMatrixContainerRef() {
				return u_modelMatrixContainer;
			}

			ShaderDataReference& getViewMatrixContainerRef() {
				return u_viewMatrixContainer;
			}

			ShaderDataReference& getPTNContainerRef() {
				return m_ptnContainer;
			}

			ShaderDataReference& getVertexContainerRef() {
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
			ShaderDataReference m_textureContainer;
			ShaderDataReference m_vertexContainer;
			ShaderDataReference m_ptnContainer;
			ShaderDataReference u_viewMatrixContainer;
			ShaderDataReference u_modelMatrixContainer;
	};

	class Sprite2DRenderer : public ComponentProcessor
	{
		public: 
			Sprite2DRenderer(GameEngine* engine);


			bool Init() override;
			void Stop() override;

			void setActiveCamera(ComponentRef<Camera2DComponent>& handle);
			void Prepare();

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

			void prepare_legacy();
			void prepare_new();

			void buildBatchInstances();


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