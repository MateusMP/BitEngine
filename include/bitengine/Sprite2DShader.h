#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.h"
#include "VertexData.h"
#include "glSystem.h"

#include "Sprite.h"

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

			void LoadViewMatrix(const glm::mat4& matrix);

			// Overrides

			GLuint CreateVAO(GLuint* outVBO) override;

		public:
			/// @brief Vertex with position and texture data only
			/// 
			class Vertex{
			public:
				Vertex(){}
				Vertex(int x, int y, float u, float v)
				{
					position.x = (float)x;
					position.y = (float)y;
					uv.u = u;
					uv.v = v;
				}
				Vertex(float x, float y, float u, float v)
				{
					position.x = x;
					position.y = y;
					uv.u = u;
					uv.v = v;
				}
				Vertex(const glm::vec2& _pos, const glm::vec2& _uv)
				{
					position.x = _pos.x;
					position.y = _pos.y;
					uv.u = _uv.x;
					uv.v = _uv.y;
				}

				VertexData::Position position;
				VertexData::UV uv;
			};

		protected:
			void BindAttributes() override;

			void RegisterUniforms() override;

			void OnBind() override;

		protected:
			// Locations
			int32 u_texDiffuseHdl;
			int32 u_viewMatrixHdl;

		private:
			// Uniform data
			glm::mat4 u_viewMatrix;



		public:
			void* operator new(size_t size){ return _aligned_malloc(size, 16); }
			void operator delete(void* mem) { return _aligned_free(mem); }
			
};


}