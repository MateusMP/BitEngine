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
			GL_CHECK(glEnableVertexAttribArray(attrIndex));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
			GL_CHECK(glVertexAttribPointer(attrIndex, size, dataType, normalized, stride, (void*)offset));
			GL_CHECK(glVertexAttribDivisor(attrIndex, divisor));
		}

		static void deleteVaos(u32 num, GLuint* _array) {
			GL_CHECK(glDeleteVertexArrays(num, _array));
		}

		static void bindShaderProgram(GLuint program) {
			GL_CHECK(glUseProgram(program));
		}
	};

	struct VBOAttrib
	{
		VBOAttrib()
			: id(0), size(0), type(0)
		{}
		VBOAttrib(const VBOAttrib& copy)
			: id(copy.id), size(copy.size), type(copy.type), dataType(copy.dataType), normalized(copy.normalized), stride(copy.stride), offset(copy.offset)
		{
		}

		GLuint id;
		GLint size;
		GLenum type;
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