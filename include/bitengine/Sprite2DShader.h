#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.h"
#include "VertexData.h"
#include "glSystem.h"

namespace BitEngine
{
	/// Draws 2D sprites using 2D coordinates
	/// 
	class Sprite2DShader :
		public ShaderProgram
	{
		public:
			class Sprite2DBatch;

			// static const uint32 ATTR_VERTEX_POS = 0;
			static const uint32 ATTR_VERTEX_TEX = 0; // 0, 1, 2, 3
			static const uint32 ATTR_SPRITE_OFF = 4; // offset and size
			static const uint32 ATTR_MODEL_MAT = 5;

			static const uint32 NUM_VBOS = 2;
			static const uint32 VBO_VERTEXDATA = 0;
			static const uint32 VBO_MODELMAT = 1;

			static const uint32 TEXTURE_DIFFUSE = 0;

		public:
			Sprite2DShader();
			~Sprite2DShader();

			// Compile shaders
			int Init() override;

			void LoadViewMatrix(const glm::mat4& matrix);

			// Overrides

			GLuint CreateVAO(GLuint* outVBO);

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