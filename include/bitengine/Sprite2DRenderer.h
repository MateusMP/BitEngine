#pragma once

#include <unordered_map>

#include "Sprite2DShader.h"
#include "EntitySystem.h"

#include "Sprite2DComponent.h"
#include "Camera2DProcessor.h"


namespace BitEngine{

class Sprite2DRenderer : public ComponentHolderProcessor
{
	public:
		Sprite2DRenderer(EntitySystem *sys, Transform2DProcessor* t2p, Camera2DProcessor* camera2Dprocessor);
		~Sprite2DRenderer();
		
		Sprite2DShader* getShader();
				
	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		std::vector<ComponentHandle>& getComponents() override;

	private:
		class BatchRenderer
		{
			public:
				BatchRenderer(SpriteManager* sprMng, Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS]);
				~BatchRenderer();

				void begin();

				void addComponent(Transform2DComponent *t, Sprite2DComponent* c);
			
				void render();

			public:
				struct Batch{
					Batch(uint32 _offset, int _nI, uint32 _texture, bool tr)
						: offset(_offset), nItems(_nI), texture(_texture), transparent(tr)
					{}

					uint32 offset;
					int nItems;
					uint32 texture;
					bool transparent;
				};

				struct RenderingComponent{
					RenderingComponent(const Transform2DComponent* t, const Sprite2DComponent* s)
						: transform(t), spr(s) {}
					const Transform2DComponent* transform;
					const Sprite2DComponent* spr;
				};

			private:

				void sortComponents();
				void createRenderers();
				void renderBatches();

				static bool compare_Texture(const RenderingComponent& a, const RenderingComponent& b);
				static bool compare_Depth(const RenderingComponent& a, const RenderingComponent& b);
				static bool compare_InvDepth(const RenderingComponent& a, const RenderingComponent& b);
				static bool compare_TextureDepth(const RenderingComponent& a, const RenderingComponent& b);
				static bool compare_DepthTexture(const RenderingComponent& a, const RenderingComponent& b);

			private:
				SpriteManager* m_spriteManager;

				Sprite2DComponent::SORT_TYPE m_sorting;

				GLuint m_vao;
				GLuint m_vbo[Sprite2DShader::NUM_VBOS];

				std::vector<RenderingComponent> m_components; // Components used by this batch - sorted
				
				// Batches
				std::vector<Batch> batches;
		};

	private:
		bool insideScreen(const Transform2DComponent* t, const Sprite2DComponent* spr);
		void createRenderers();

	private:
		EntitySystem *es;
		Transform2DProcessor* transform2Dprocessor;
		Camera2DProcessor* camera2Dprocessor;

		SpriteManager* m_spriteManager;

		std::vector<BatchRenderer*> m_batchRenderers; // Sorted by draw order

		GLenum glMODE;

		Sprite2DShader *shader;

		ComponentCollection<Sprite2DComponent> components;
};

}