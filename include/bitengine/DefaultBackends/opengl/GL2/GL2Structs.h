#pragma once

#include <set>

#include "OpenGL2.h"

namespace BitEngine
{
	class GL2
	{
		public:

		static void bindVbo(GLuint vbo) {
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		}
		static void unbindVbo() {
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}

		static void loadBufferRange(const void* data, u32 offset, u32 len, GLenum mode) {
			const char* d = static_cast<const char*>(data);
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, nullptr, mode));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, d + offset, mode));
		}

		static void bindVao(GLuint vao) {
			GL_CHECK(glBindVertexArray(vao));
		}

		static void unbindVao() {
			GL_CHECK(glBindVertexArray(0));
		}

		static void genVao(GLsizei num, GLuint* _array) {
			GL_CHECK(glGenVertexArrays(num, _array));
		}

		static void genVbo(GLsizei num, GLuint* _array) {
			glGenBuffers(num, _array);
		}

		static void setupVbo(GLuint attrIndex, GLuint vbo, GLint size, GLenum dataType, GLboolean normalized, GLsizei stride, u32 offset, GLuint divisor) {
			bindVbo(vbo);
			GL_CHECK(glVertexAttribPointer(attrIndex, size, dataType, normalized, stride, (void*)offset));
			// GL_CHECK(glVertexAttribDivisor(attrIndex, divisor));
			GL_CHECK(glEnableVertexAttribArray(attrIndex));
		}

		static void deleteVaos(u32 num, GLuint* _array) {
			GL_CHECK(glDeleteVertexArrays(num, _array));
		}

		static void bindShaderProgram(GLuint program) {
			GL_CHECK(glUseProgram(program));
		}

		static GLenum toGLType(DataType dt)
		{
			switch (dt)
			{
			case DataType::FLOAT:
				return GL_FLOAT;
			case DataType::MAT3:
				return GL_FLOAT_MAT3;
			case DataType::MAT4:
				return GL_FLOAT_MAT4;
			case DataType::VEC2:
				return GL_FLOAT_VEC2;
			case DataType::VEC3:
				return GL_FLOAT_VEC3;
			case DataType::VEC4:
				return GL_FLOAT_VEC4;
			case DataType::TEXTURE_1D:
				return GL_SAMPLER_1D;
			case DataType::TEXTURE_2D:
				return GL_SAMPLER_2D;
			case DataType::TEXTURE_3D:
				return GL_SAMPLER_3D;
			case DataType::LONG:
				return GL_INT;
			default:
				return GL_FLOAT;
			}
		}

		static GLenum fromGLTypeToGLDataType(GLenum e)
		{
			switch (e)
			{
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
				return GL_INT;

			case GL_FLOAT_MAT4:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT2:
			case GL_FLOAT_VEC4:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC2:
			case GL_FLOAT:
				return GL_FLOAT;
			default:
				ASSERT(e && 0 && "Unexpected type");
				return e;
			}
		}

		static GLenum fromGLTypeToGLDataTypeSize(GLenum e)
		{
			switch (e)
			{
			case GL_FLOAT:
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
				return 1;
			case GL_FLOAT_MAT3:
			case GL_FLOAT_VEC3:
				return 3;
			case GL_FLOAT_MAT4:
			case GL_FLOAT_VEC4:
				return 4;
			case GL_FLOAT_MAT2:
			case GL_FLOAT_VEC2:
				return 2;
			default:
				ASSERT(e && 0 && "Unexpected type");
				return e;
			}
		}
	};

	struct VBOAttrib
	{
		VBOAttrib()
			: id(0), dataSize(0), type(0)
		{}
		VBOAttrib(const VBOAttrib& copy)
			: id(copy.id), dataSize(copy.dataSize), type(copy.type), dataType(copy.dataType), normalized(copy.normalized), stride(copy.stride), offset(copy.offset)
		{
		}

		GLuint id;
		GLenum type;
		GLint dataSize; // num of elements of dataType
		GLenum dataType;
		GLboolean normalized;
		GLsizei stride;
		int offset;

	};

	struct VBOContainer
	{
		ShaderDataReference ref;
		std::vector<VBOAttrib> attrs;
		GLsizei stride;
		GLuint divisor;
		GLuint vbo;
	};

	struct VAOContainer
	{
		GLuint vao;
		std::vector<VBOContainer> vbos;
	};

	struct UniformDefinition
	{
		struct Comparator {
			bool operator() (const UniformDefinition& lhs, const UniformDefinition& rhs) const {
				return ShaderDataReference::Hasher()(lhs.ref, rhs.ref);
			}
		};
		ShaderDataReference ref;
		GLint location;
		GLuint byteSize;
		GLenum type;
		GLint size;
		u8 instanced;
		u8 extraInfo;
		char* dataOffset; // as an offset
	};


	// Contain multiples UniformDefinition
	struct UniformContainer
	{
		UniformContainer(const ShaderDataReference& _ref, u32 _instance)
			: ref(_ref), instance(_instance)
		{
		}

		ShaderDataReference ref;
		u32 instance;
		u32 stride;
		std::vector<UniformDefinition> defs;
	};


	struct UniformHolder
	{
		std::vector<UniformContainer> containers;
	};
}