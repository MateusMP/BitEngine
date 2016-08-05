#pragma once

#include <set>

#include "OpenGL2.h"

namespace BitEngine
{
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
		ShaderDataDefinition::DefinitionReference ref;
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
				return ShaderDataDefinition::DefinitionReference::Hasher()(lhs.ref, rhs.ref);
			}
		};
		ShaderDataDefinition::DefinitionReference ref;
		GLint location;
		GLuint byteSize;
		GLenum type;
		GLint size;
		u8 instanced;
		u8 extraInfo;
		char* dataOffset; // as an offset
	};

	// Contain multiples UniformDefinition
	struct UniformContainer : std::set<UniformDefinition, UniformDefinition::Comparator>
	{
		u16 calculateMaxDataSize(int instanced) const {
			u16 maxDataSize = 0;
			for (auto& it = begin(); it != end(); ++it)
			{
				if (it->instanced == instanced)
				{
					maxDataSize += it->byteSize;
				}
			}

			return maxDataSize;
		}
	};

	///
	struct UniformData
	{
		UniformData(UniformDefinition def, char* address)
			: unif(def)
		{
			unif.dataOffset = address;
		}
		UniformDefinition unif;
	};

}