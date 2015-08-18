#pragma once

#include "Sprite2DShader.h"
#include "Sprite.h"

namespace BitEngine{

	class Sprite2DBatchRenderer
	{
	public:
		Sprite2DBatchRenderer(Sprite2DShader::RendererVersion renderer);
		~Sprite2DBatchRenderer();

		void Begin();
		void End();

		void setSorting(bool sort) { m_sortActive = sort; }

		void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth);

		void Render();

	public:
		struct Batch{
			Batch(uint32 _offset, int _nI, uint32 _texture, bool tr)
				: offset(_offset), nItems(_nI), texture(_texture), transparent(tr)
			{}

			uint32 offset;
			int nItems;
			uint32 texture;
			bool transparent;

			GLuint iVAO;
			GLuint* iVBO;
		};

		struct RenderingElement {
			RenderingElement(GLuint _depth, const Sprite* _sprite, const glm::mat3* _modelMatrix)
				: depth(_depth), sprite(_sprite), modelMatrix(_modelMatrix)
			{}

			GLuint depth;
			const Sprite* sprite;
			const glm::mat3* modelMatrix;
		};

	private:

		void sortComponents();
		void createRenderers();
		void renderBatches();

		void createRenderersGL2();
		void createRenderersGL4GL3();

		void renderGL4();
		void renderGL3();
		void renderGL2();

		static bool compare_DepthTexture(const RenderingElement& a, const RenderingElement& b);

	private:
		Sprite2DShader::RendererVersion RENDERER_VERSION;
		bool m_sortActive;

		//GLuint m_vao;
		//GLuint m_vbo[Sprite2DShader::NUM_VBOS];

		//std::vector<RenderingComponent> m_components; // Components used by this batch - sorted
		std::vector<RenderingElement> m_elements;

		// Batches
		std::vector<Batch> batches;
			
		std::vector<Sprite2Dinstanced_VAOinterleaved> m_interVAOs; // GL3 and GL4
		std::vector<Sprite2Dbasic_VAOinterleaved> m_basicVAOs; // GL2
	};

}