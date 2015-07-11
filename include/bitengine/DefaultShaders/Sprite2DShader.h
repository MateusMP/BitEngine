#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "..\ShaderProgram.h"
#include "..\VertexData.h"
#include "..\glSystem.h"

#include "..\Sprite.h"

namespace BitEngine
{
	/// Draws 2D sprites using 2D coordinates
	/// 
	class Sprite2DShader :
		public ShaderProgram
	{
		public:
			class Sprite2DBatch;

			static const uint32 ATTR_VERTEX_POS = 0;
			static const uint32 ATTR_VERTEX_TEX = 1;

			static const uint32 NUM_VBOS = 1;

			static const uint32 TEXTURE_DIFFUSE = 0;

		public:
			Sprite2DShader();
			~Sprite2DShader();

			// Compile shaders
			int init();

			void LoadViewMatrix(glm::mat4& matrix);

			Sprite2DBatch* Create2DBatchRenderer(IBatchRenderer::BATCH_MODE mode);

		protected:
			void BindAttributes() override;

			void RegisterUniforms() override;

			void OnBind() override;

			GLuint CreateVAO(GLuint* outVBO) override;

		
		protected:
			// Locations
			int32 u_texDiffuseHdl;
			int32 u_viewMatrixHdl;

			///
			/// @brief Vertex with only position and texture data
			/// 
			class Vertex{
			public:
				Vertex(){}
				Vertex(glm::vec2 _pos, glm::vec2 _uv)
				{
					position.x = _pos.x;
					position.y = _pos.y;
					uv.u = _uv.x;
					uv.v = _uv.y;
				}

				VertexData::Position position;
				VertexData::UV uv;
			};

		private:
			glm::mat4 u_viewMatrix;

		/// ============================ RENDERERS ============================

		public:
			class Sprite2DBatch : IBatchRenderer
			{
				public:
					friend class Sprite2DShader;
					enum class SORT_TYPE{
						NOONE,
						BY_TEXTURE_ONLY,
						BY_DEPTH_ONLY,
						BY_INVDEPTH_ONLY,
						BY_TEXTURE_DEPTH,
						BY_DEPTH_TEXTURE
					};

					Sprite2DBatch(BATCH_MODE _mode);
					~Sprite2DBatch();

					void setSortingType(SORT_TYPE type);

					/** @param Draws sprite on 2D world coordinates
					*/
					void DrawSprite(const glm::vec2& pos, const Sprite& sprite, int width, int height, float depth = 0);

					void begin() override;
					void end() override;

					void Render() override;


				private:
					class Glyph{
					public:
						Glyph(const glm::vec2& _pos, const Sprite& _sprite, int width, int height, float _depth);
						Glyph(const glm::vec2& _pos, const Sprite& _sprite, float _depth);

						Vertex topleft;
						Vertex bottomleft;
						Vertex topright;
						Vertex bottomright;

						uint32 textureID;
						float depth;
					};

					class Renderer
					{
					public:
						Renderer(uint32 _offset, int _nVertices, uint32 _texture)
							: offset(_offset), nVertices(_nVertices), texture(_texture)
						{}

						uint32 offset;
						int nVertices;
						uint32 texture;
					};


				private:
					GLuint m_vao;
					GLuint m_vbo[NUM_VBOS];

					std::vector<Glyph*> m_glyphs;
					std::vector<Glyph> m_glyphsData;
					std::vector<Renderer> batchRenderers;

					SORT_TYPE m_sort;
					BATCH_MODE m_mode;
					GLenum glMODE;

					void sortGlyphs();
					void createRenderers();

					static bool compare_Texture(Glyph* a, Glyph* b);
					static bool compare_Depth(Glyph* a, Glyph* b);
					static bool compare_InvDepth(Glyph* a, Glyph* b);
					static bool compare_TextureDepth(Glyph* a, Glyph* b);
					static bool compare_DepthTexture(Glyph* a, Glyph* b);
			};

			
};


}