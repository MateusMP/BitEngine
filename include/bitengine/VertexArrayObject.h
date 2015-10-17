#pragma once

#include <vector>

#include "EngineLoggers.h"
#include "TypeDefinition.h"
#include "Graphics.h"

namespace BitEngine{
	
	class VertexAttribute
	{
	public:
		VertexAttribute(uint32 _type, uint32 _count, bool _normalized, GLuint _divisor)
			: type(_type), count(_count), normalized(_normalized), divisor(_divisor)
		{
			uint32 bytes = 4;
			if (type == GL_FLOAT) bytes = sizeof(GLfloat);
			else if (type == GL_BYTE) bytes = sizeof(GLbyte);
			else if (type == GL_UNSIGNED_BYTE) bytes = sizeof(GLbyte);
			else if (type == GL_SHORT) bytes = sizeof(GLshort);
			else if (type == GL_UNSIGNED_SHORT) bytes = sizeof(GLshort);
			else if (type == GL_INT) bytes = sizeof(GLint);
			else if (type == GL_UNSIGNED_INT) bytes = sizeof(GLint);
			else {
				LOGTO(Error) << "Invalid VertexAttribute" << endlog;
			}

			sizeBytes = count * bytes;
		}
		
		uint32 type;
		uint32 count;
		bool normalized;
		GLuint divisor;
		uint32 sizeBytes;
	};

	template <typename T, int N> char(&ctadim(T(&)[N]))[N];

#define compile_time_array_dim(x) (sizeof(BitEngine::ctadim(x)))


#define DECLARE_VERTEXDATA(name, numAttrs)									\
	class name {															\
		public:																\
		static const uint32 NUM_ATTRIBUTES = numAttrs;						\
		static const BitEngine::VertexAttribute attributes[NUM_ATTRIBUTES];	\
		static const BitEngine::VertexAttribute& getAttribute(int index) {	\
			static const BitEngine::VertexAttribute v[] = {

#define ADD_ATTRIBUTE(type, count, normalized, divisor)				\
				BitEngine::VertexAttribute(type, count, normalized, divisor)

#define END_ATTRIBUTES()													\
					};														\
			static_assert(compile_time_array_dim(v) == NUM_ATTRIBUTES,		\
			"Number of attributes does not match with declaration");		\
			return v[index];												\
				};

#define DECLARE_DATA_STRUCTURE()					\
				struct Data{

#define ADD_MEMBER_ARRAY(type, name, arraySize)		\
			type name[arraySize];

#define ADD_MEMBER(type, name)			\
			type name;

#define END_VERTEX_DATA()				\
		};	};

	/*
	EXAMPLE:
		DECLARE_VERTEXDATA(Sprite2D_VDinstancedVertices, 5)
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
	*/

	class IVertexArrayBuffer
	{public:
		static void UnbindBuffer(){
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	};

	template<bool interleaved, typename VertexData>
	class VertexArrayBuffer
	{
	};

	// ------------------------------------------------------------------------------
	// INTERLEAVED
	// ------------------------------------------------------------------------------
	template<typename VertexData>
	class VertexArrayBuffer<true, VertexData>
	{
	public:
		VertexArrayBuffer()
			: vbo(0)
		{}
		~VertexArrayBuffer(){
			
		}

		void DestroyBuffer()
		{
			glDeleteBuffers(1, &vbo);
		}

		void CreateBuffer(uint32 &attributes)
		{
			// Creates VBO
			glGenBuffers(1, &vbo);
			if (vbo == 0){
				LOGTO(Error) << "VertexBuffer: Could not create VBO." << endlog;
				return;
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			//uint32 strideSize = 0;
			//for (int i = 0; i < VertexData::NUM_ATTRIBUTES; ++i){
			//	const VertexAttribute& attr = VertexData::getAttribute(i);
			//	strideSize += attr.sizeBytes;
			//}


			LOGTO(Verbose) << "VBO: " << vbo << endlog;
			// Attributes
			uint32 accumOffset = 0;
			for (int i = 0; i < VertexData::NUM_ATTRIBUTES; ++i)
			{
				uint32 attrID = attributes + i;
				const VertexAttribute& attr = VertexData::getAttribute(i);

				glEnableVertexAttribArray(attrID);
				glVertexAttribPointer(attrID, attr.count, attr.type, attr.normalized, sizeof(VertexData::Data), (void*)(accumOffset));
				glVertexAttribDivisor(attrID, attr.divisor);

				LOGTO(Verbose) << "Defining attribute " << attrID<<"("<<i<<")" << ": count: " << attr.count <<
									", type: " << attr.type << ", normal: " << attr.normalized << 
									", size: " << sizeof(VertexData::Data) << "(" << attr.sizeBytes << ")" << ", offset:" << accumOffset << endlog;

				accumOffset += attr.sizeBytes;
			}
			attributes += VertexData::NUM_ATTRIBUTES;

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			elementSize = accumOffset;
		}

		void BindBuffer()
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
		}

		void LoadBuffer(const typename VertexData::Data* data, uint32 numElements, uint32 mode = GL_DYNAMIC_DRAW)
		{
			glBufferData(GL_ARRAY_BUFFER, elementSize * numElements, nullptr, mode);
			glBufferData(GL_ARRAY_BUFFER, elementSize * numElements, data, mode);
		}

	private:
		GLuint vbo;
		uint32 elementSize;
	};

	// ------------------------------------------------------------------------------
	// NOT INTERLEAVED
	// -----------------------------------------------------------------------------
	// Each vertex data has it's own vbo
	template<typename VertexData>
	class VertexArrayBuffer<false, VertexData>
	{
	public:
		VertexArrayBuffer()
		{}
		~VertexArrayBuffer(){
		}

		void DestroyBuffer()
		{
			glDeleteBuffers(VertexData::NUM_ATTRIBUTES, vbo);
		}

		void CreateBuffer(uint32 &attributes)
		{
			// Creates VBO
			glGenBuffers(VertexData::NUM_ATTRIBUTES, vbo);
			if (vbo[0] == 0){
				LOGTO(Error) << "VertexBuffer: Could not create VBOs." << endlog;
				return;
			}

			// Attributes
			uint32 accumOffset = 0;
			for (int i = 0; i < VertexData::NUM_ATTRIBUTES; ++i)
			{
				uint32 attrID = attributes + i;
				glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);

				const VertexAttribute& attr = VertexData::getAttribute(i);

				glEnableVertexAttribArray(attrID);
				glVertexAttribPointer(attrID, attr.count, attr.type, attr.normalized, attr.sizeBytes, 0);
				glVertexAttribDivisor(attrID, attr.divisor);
			}

			attributes += VertexData::NUM_ATTRIBUTES;
			IVertexArrayBuffer::UnbindBuffer();
		}

		template<int Attribute>
		void BindBuffer()
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo[Attribute]);
		}

		template<typename Type>
		void LoadBuffer(const Type* data, uint32 size, uint32 mode = GL_DYNAMIC_DRAW)
		{
			glBufferData(GL_ARRAY_BUFFER, sizeof(Type) * size, nullptr, mode);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Type) * size, data, mode);
		}

	private:
		GLuint vbo[VertexData::NUM_ATTRIBUTES];
	};


	template<typename T>
	class VertexArrayObject_base
	{
		public:
			T vbo;
		protected:
			bool CreateBuffer(uint32 &attributes){
				vbo.CreateBuffer(attributes);
				return true;
			}

			bool DestroyBuffer(){
				vbo.DestroyBuffer();
				return true;
			}

	};
	
	template <typename T>
	using IVBO = VertexArrayObject_base<VertexArrayBuffer<true, T>>;
	template <typename T>
	using SVBO = VertexArrayObject_base<VertexArrayBuffer<false, T>>;

	class IVertexArrayObject
	{
	public:
		static void Unbind()
		{
			glBindVertexArray(0);
		}
	};

	template<typename ... B>
	class VertexArrayObject 
		: public B...
	{
	public:
		void Destroy()
		{
			glDeleteVertexArrays(1, &vao);
			bool b[] = { this->B::DestroyBuffer()... };
		}
		
		bool Create()
		{
			glGenVertexArrays(1, &vao);
			if (vao == 0){
				LOGTO(Error) << "Sprite2DBatch: Could not create VAO." << endlog;
				return false;
			}

			LOGTO(Verbose) << "VAO: " << vao << endlog;
			Bind();

			uint32 attributes = 0;
			bool b[] = { this->B::CreateBuffer(attributes)... };

			IVertexArrayObject::Unbind();

			return true;
		}

		void Bind()
		{
			glBindVertexArray(vao);
		}

	private:
		GLuint vao;
	};

}