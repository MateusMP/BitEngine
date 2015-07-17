#pragma once

#include <unordered_map>

#include "Sprite2DShader.h"
#include "EntitySystem.h"

#include "Sprite2DComponent.h"
#include "Camera2DProcessor.h"


namespace BitEngine{

class Sprite2DRenderer : public ComponentHolderProcess<Sprite2DComponent>
{
	public:
		Sprite2DRenderer(Camera2DProcessor* camera2Dprocessor);
		~Sprite2DRenderer();
		
		Sprite2DShader* getShader();
				
	private:
		bool Init() override;
		void Process() override;

		ComponentHandle CreateComponent() override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		std::vector<Sprite2DComponent*>& getComponents() override;

	private:
		class BatchRenderer
		{
			public:
				BatchRenderer(Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS]);
				~BatchRenderer();

				void begin();

				void addComponent(Sprite2DComponent* c);
			
				void render();

			public:
				struct Batch{
					Batch(uint32 _offset, int _nVertices, uint32 _texture)
						: offset(_offset), nVertices(_nVertices), texture(_texture)
					{}

					uint32 offset;
					int nVertices;
					uint32 texture;
				};

			private:

				void sortComponents();
				void createRenderers();
				void renderBatches();

				static bool compare_Texture(Sprite2DComponent* a, Sprite2DComponent* b);
				static bool compare_Depth(Sprite2DComponent* a, Sprite2DComponent* b);
				static bool compare_InvDepth(Sprite2DComponent* a, Sprite2DComponent* b);
				static bool compare_TextureDepth(Sprite2DComponent* a, Sprite2DComponent* b);
				static bool compare_DepthTexture(Sprite2DComponent* a, Sprite2DComponent* b);

			private:
				Sprite2DComponent::SORT_TYPE m_sorting;

				GLuint m_vao;
				GLuint m_vbo[Sprite2DShader::NUM_VBOS];

				std::vector<Sprite2DComponent*> m_components; // Components used by this batch - sorted
				
				// Batches
				std::vector<Batch> batches;
		};

	private:
		void createRenderers();

	private:
		Camera2DProcessor* camera2Dprocessor;

		std::vector<BatchRenderer*> m_batchRenderers; // Sorted by draw order

		GLenum glMODE;

		Sprite2DShader *shader;

		ComponentCollection<Sprite2DComponent> components;


	public:
};

}