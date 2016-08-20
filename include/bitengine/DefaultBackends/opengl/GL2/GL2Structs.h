#pragma once

#include <set>

#include "OpenGL2.h"
#include "GL2Impl.h"

namespace BitEngine
{
	struct VBOAttrib
	{
		VBOAttrib()
			: id(0), dataSize(0), type(0)
		{}
		VBOAttrib(const VBOAttrib& copy)
			: id(copy.id), type(copy.type), size(copy.size),
			  dataType(copy.dataType), dataSize(copy.dataSize),
			  normalized(copy.normalized), stride(copy.stride), offset(copy.offset),
			  divisor(copy.divisor), name(copy.name)
		{
		}

		GLuint id;
		GLenum type; // like VEC2
        GLint size; // the number of instances of the type, like vec2 x[5] would be 5
		GLenum dataType; // like FLOAT
		GLint dataSize; // num of elements of dataType
		GLboolean normalized;
		GLsizei stride;
		int offset;
        u32 divisor;
        std::string name;
	};

	struct VBOContainer
	{
		ShaderDataReference ref;
		std::vector<const VBOAttrib*> attrs;
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
		u8 extraInfo; // When used for samplers, says the texture unit to use.
		u32 dataOffset; // as an offset
        
        std::string name;
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
		std::vector<const UniformDefinition*> defs;
	};


	struct UniformHolder
	{
		std::vector<UniformContainer> containers;
	};
}