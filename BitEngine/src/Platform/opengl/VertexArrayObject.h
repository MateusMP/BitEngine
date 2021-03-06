#pragma once

#include <vector>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Common/MathUtils.h"
#include "BitEngine/Core/Logger.h"

#include "Platform/opengl/OpenGL.h"

namespace BitEngine {

class VertexAttribute {
public:
    VertexAttribute(u32 _type, u32 _count, bool _normalized, GLuint _divisor)
        : type(_type)
        , count(_count)
        , normalized(_normalized)
        , divisor(_divisor)
    {
        u32 bytes = 4;
        if (type == GL_FLOAT)
            bytes = sizeof(GLfloat);
        else if (type == GL_BYTE)
            bytes = sizeof(GLbyte);
        else if (type == GL_UNSIGNED_BYTE)
            bytes = sizeof(GLbyte);
        else if (type == GL_SHORT)
            bytes = sizeof(GLshort);
        else if (type == GL_UNSIGNED_SHORT)
            bytes = sizeof(GLshort);
        else if (type == GL_INT)
            bytes = sizeof(GLint);
        else if (type == GL_UNSIGNED_INT)
            bytes = sizeof(GLint);
        else {
            LOG(EngineLog, BE_LOG_ERROR) << "Invalid VertexAttribute";
        }

        sizeBytes = count * bytes;
    }

    u32 type;
    u32 count;
    bool normalized;
    GLuint divisor;
    u32 sizeBytes;
};

#define DECLARE_VERTEXDATA(name, numAttrs)                                  \
    class name {                                                            \
    public:                                                                 \
        static constexpr u32 NUM_ATTRIBUTES = numAttrs;                     \
        static const BitEngine::VertexAttribute attributes[NUM_ATTRIBUTES]; \
        static const BitEngine::VertexAttribute& getAttribute(int index)    \
        {                                                                   \
        static const BitEngine::VertexAttribute v[] = {

#define ADD_ATTRIBUTE(type, count, normalized, divisor) \
    BitEngine::VertexAttribute(type, count, normalized, divisor)

#define END_ATTRIBUTES()                                         \
    }                                                            \
    ;                                                            \
    static_assert(BitEngine::CT_array_size(v) == NUM_ATTRIBUTES, \
        "Number of attributes does not match with declaration"); \
    return v[index];                                             \
    }                                                            \
    ;

#define DECLARE_DATA_STRUCTURE() \
    struct Data {

#define ADD_MEMBER_ARRAY(type, name, arraySize) \
    type name[arraySize];

#define ADD_MEMBER(type, name) \
    type name;

#define END_VERTEX_DATA() \
    }                     \
    ;                     \
    }                     \
    ;

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

enum ArrayBufferType {
    STANDARD,
    INTERLEAVED,
};

template <ArrayBufferType type, typename VertexData>
class ArrayBuffer {
public:
    typedef VertexData vd;

    bool CreateBuffer(u32& attributes);
    bool DestroyBuffer();
};

template <typename T>
class BufferVector : public std::vector<typename T::Data> {
    typedef typename T::Data vd;
};

// ------------------------------------------------------------------------------
// INTERLEAVED
// ------------------------------------------------------------------------------
template <typename VertexData>
class ArrayBuffer<INTERLEAVED, VertexData> {
public:
    ArrayBuffer()
        : vbo(0)
    {
    }
    ~ArrayBuffer()
    {
    }

    bool DestroyBuffer()
    {
        glDeleteBuffers(1, &vbo);
        return true;
    }

    bool CreateBuffer(u32& attributes)
    {
        // Creates VBO
        glGenBuffers(1, &vbo);
        if (vbo == 0) {
            LOG(EngineLog, BE_LOG_ERROR) << "VertexBuffer: Could not create VBO.";
            return false;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //u32 strideSize = 0;
        //for (int i = 0; i < VertexData::NUM_ATTRIBUTES; ++i){
        //	const VertexAttribute& attr = VertexData::getAttribute(i);
        //	strideSize += attr.sizeBytes;
        //}

        LOG(EngineLog, BE_LOG_VERBOSE) << "VBO: " << vbo;
        // Attributes
        u32 accumOffset = 0;
        for (u32 i = 0; i < VertexData::NUM_ATTRIBUTES; ++i) {
            u32 attrID = attributes + i;
            const VertexAttribute& attr = VertexData::getAttribute(i);

            glEnableVertexAttribArray(attrID);
            glVertexAttribPointer(attrID, attr.count, attr.type, attr.normalized, sizeof(typename VertexData::Data), (void*)(accumOffset));
            glVertexAttribDivisor(attrID, attr.divisor);

            LOG(EngineLog, BE_LOG_VERBOSE) << "Defining attribute " << attrID << "(" << i << ")"
                                           << ": count: " << attr.count << ", type: " << attr.type << ", normal: " << attr.normalized << ", size: " << sizeof(typename VertexData::Data) << "(" << attr.sizeBytes << ")"
                                           << ", offset:" << accumOffset;

            accumOffset += attr.sizeBytes;
        }
        attributes += VertexData::NUM_ATTRIBUTES;

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        elementSize = accumOffset;

        return true;
    }

    void BindBuffer()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void LoadBufferRange(const BufferVector<VertexData>& data, u32 offset, u32 len, u32 mode = GL_DYNAMIC_DRAW)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * len, nullptr, mode);
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * len, data.data() + offset, mode);
    }

    void LoadBuffer(const BufferVector<VertexData>& data, u32 mode = GL_DYNAMIC_DRAW)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * data.size(), nullptr, mode);
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * data.size(), data.data(), mode);
    }

    void BindAndLoadBufferRange(const BufferVector<VertexData>& data, u32 offset, u32 len, u32 mode = GL_DYNAMIC_DRAW)
    {
        BindBuffer();
        LoadBufferRange(data, offset, len, mode);
    }

    void BindAndLoadBuffer(const BufferVector<VertexData>& data, u32 mode = GL_DYNAMIC_DRAW)
    {
        BindBuffer();
        LoadBuffer(data, mode);
    }

private:
    GLuint vbo;
    u32 elementSize;
};
// ------------------------------------------------------------------------------
// NOT INTERLEAVED
// -----------------------------------------------------------------------------
// Each vertex data has it's own vbo
template <typename VertexData>
class ArrayBuffer<STANDARD, VertexData> {
public:
    ArrayBuffer()
    {
    }
    ~ArrayBuffer()
    {
    }

    bool DestroyBuffer()
    {
        glDeleteBuffers(VertexData::NUM_ATTRIBUTES, vbo);
        return true;
    }

    bool CreateBuffer(u32& attributes)
    {
        // Creates VBO
        glGenBuffers(VertexData::NUM_ATTRIBUTES, vbo);
        if (vbo[0] == 0) {
            LOG(EngineLog, BE_LOG_ERROR) << "VertexBuffer: Could not create VBOs.";
            return false;
        }

        // Attributes
        u32 accumOffset = 0;
        for (int i = 0; i < VertexData::NUM_ATTRIBUTES; ++i) {
            u32 attrID = attributes + i;
            glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);

            const VertexAttribute& attr = VertexData::getAttribute(i);

            glEnableVertexAttribArray(attrID);
            glVertexAttribPointer(attrID, attr.count, attr.type, attr.normalized, attr.sizeBytes, 0);
            glVertexAttribDivisor(attrID, attr.divisor);
        }

        attributes += VertexData::NUM_ATTRIBUTES;
        //IVertexArrayBuffer::UnbindBuffer();
        return true;
    }

    template <int Attribute>
    void BindBuffer()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[Attribute]);
    }

    void LoadBufferRange(const BufferVector<VertexData>& data, u32 offset, u32 len, u32 mode = GL_DYNAMIC_DRAW)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * len, nullptr, mode);
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * len, data.data() + offset, mode);
    }

    void LoadBuffer(const BufferVector<VertexData>& data, u32 mode = GL_DYNAMIC_DRAW)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * data.size(), nullptr, mode);
        glBufferData(GL_ARRAY_BUFFER, sizeof(typename VertexData::Data) * data.size(), data.data(), mode);
    }

    void BindAndLoadBufferRange(const BufferVector<VertexData>& data, u32 offset, u32 len, u32 mode = GL_DYNAMIC_DRAW)
    {
        BindBuffer();
        LoadBufferRange(data, offset, len, mode);
    }

    void BindAndLoadBuffer(const BufferVector<VertexData>& data, u32 mode = GL_DYNAMIC_DRAW)
    {
        BindBuffer();
        LoadBuffer(data, mode);
    }

private:
    GLuint vbo[VertexData::NUM_ATTRIBUTES];
};

template <typename T>
using IVBO = ArrayBuffer<INTERLEAVED, T>;
template <typename T>
using SVBO = ArrayBuffer<STANDARD, T>;

class VertexArrayHelper {
public:
    static void UnbindVBO()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    static void UnbindVAO()
    {
        glBindVertexArray(0);
    }
};

// Template parameters should be IVBOs and/or SVBOs
//template< template <typename VD> typename ... B>
//template<typename SVBO<S> ... S1, IVBO<I> ... I1>

template <typename... B>
class VertexArrayObject
    : public B... {
    //template<typename VertexData>
    //using B<VertexData>::BindAndLoadBuffer();
public:
    void Destroy()
    {
        glDeleteVertexArrays(1, &vao);
        bool b[] = { this->B::DestroyBuffer()... };
        if (b[0]) // remove "b" not used warning
            b[0] = b[0];
    }

    bool Create()
    {
        glGenVertexArrays(1, &vao);
        if (vao == 0) {
            LOG(EngineLog, BE_LOG_ERROR) << "VertexArrayObject: Could not create VAO.";
            return false;
        }

        LOG(EngineLog, BE_LOG_VERBOSE) << "VAO: " << vao;
        Bind();

        u32 attributes = 0;
        bool b[] = { this->B::CreateBuffer(attributes)... };
        if (b[0]) // remove "b" not used warning
            b[0] = b[0];

        VertexArrayHelper::UnbindVAO();

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
