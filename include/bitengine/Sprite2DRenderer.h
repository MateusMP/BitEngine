#pragma once

#include "Sprite2DShader.h"
#include "Sprite.h"

namespace BitEngine{

	class Sprite2DRenderer{
	public:
		Sprite2DRenderer();
		~Sprite2DRenderer();

		bool Init();

		Sprite2DShader* getShader();

		/**
		* Clear renderer queue
		*/
		void Begin();

		/**
		* Add sprite to the render queue
		*/
		void addToRender(SpriteSortType sortmode, const Sprite* sprite, const glm::mat3* modelMatrix, int depth);

		/**
		* Marks the render as finished so the renderer can
		* prepare the data for rendering
		*/
		void End();

		/**
		* Render queue
		*/
		void Render();

	public:
		
		class BatchRenderer
		{
		public:
			BatchRenderer(SpriteSortType s);
			~BatchRenderer();

			void begin();
			void end();

			void drawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth);

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

			void renderGL4();
			void renderGL3();

			static bool compare_DepthTexture(const RenderingElement& a, const RenderingElement& b);

		private:
			SpriteSortType m_sorting;

			//GLuint m_vao;
			//GLuint m_vbo[Sprite2DShader::NUM_VBOS];

			//std::vector<RenderingComponent> m_components; // Components used by this batch - sorted
			std::vector<RenderingElement> m_elements;

			// Batches
			std::vector<Batch> batches;

			std::vector<Sprite2DShader::Vao> m_VAOS;

			std::vector<Sprite2Dinstanced_VAOinterleaved> m_interVAOs;
		};

	private:
		std::vector<BatchRenderer*> m_batchRenderers; // Sorted by SpriteSortType

		Sprite2DShader *shader;
	};


}