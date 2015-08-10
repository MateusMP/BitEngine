#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.h"
#include "glSystem.h"

#include "VertexArrayObject.h"

namespace BitEngine
{
	/// Draws 2D sprites using 2D coordinates
	/// 
	DECLARE_VERTEXDATA(Sprite2Dinstanced_VDVertices, 5)
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 1)
	END_ATTRIBUTES()
	DECLARE_DATA_STRUCTURE()
		ADD_MEMBER_ARRAY(glm::vec2, tex_coord, 4)
		ADD_MEMBER(glm::vec4, size_offset)
	END_VERTEX_DATA()

	DECLARE_VERTEXDATA(Sprite2Dinstanced_VDModelMatrix, 3)
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
	END_ATTRIBUTES()
	DECLARE_DATA_STRUCTURE()
		ADD_MEMBER(glm::mat3, modelMatrix)
	END_VERTEX_DATA()

	typedef VertexArrayObject 
		<SVBO<Sprite2Dinstanced_VDVertices>, SVBO<Sprite2Dinstanced_VDModelMatrix>> Sprite2Dinstanced_VAOseparated;
	typedef VertexArrayObject
		<IVBO<Sprite2Dinstanced_VDVertices>, IVBO<Sprite2Dinstanced_VDModelMatrix>> Sprite2Dinstanced_VAOinterleaved;

	class Sprite2DShader :
		public ShaderProgram
	{
		public:
			// static const uint32 ATTR_VERTEX_POS = 0;
			/*static const uint32 ATTR_VERTEX_TEX = 0; // 0, 1, 2, 3
			static const uint32 ATTR_SPRITE_OFF = 4; // offset and size
			static const uint32 ATTR_MODEL_MAT = 5;

			static const uint32 NUM_VBOS = 2;
			static const uint32 VBO_VERTEXDATA = 0;
			static const uint32 VBO_MODELMAT = 1;*/

			static const uint32 TEXTURE_DIFFUSE = 0;

		public:
			Sprite2DShader();
			~Sprite2DShader();

			// Compile shaders
			int Init() override;

			void LoadViewMatrix(const glm::mat4& matrix);

			// Overrides

			struct Vao{
				//GLuint VAO;
				//GLuint VBO[NUM_VBOS];

				void free(){
					//glDeleteVertexArrays(1, &VAO);
					//glDeleteBuffers(NUM_VBOS, VBO);
				}
			};
			static Vao CreateVAO();

			//
			
			//

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