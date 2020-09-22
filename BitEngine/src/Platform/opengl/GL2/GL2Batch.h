#pragma once

#include <unordered_map>

#include "Platform/opengl/GL2/OpenGL2.h"

namespace BitEngine {
struct VBOAttrib {
    VBOAttrib()
        : id(0)
        , type(0)
        , dataSize(0)
    {
    }
    VBOAttrib(const VBOAttrib& copy)
        : id(copy.id)
        , type(copy.type)
        , size(copy.size)
        , dataType(copy.dataType)
        , dataSize(copy.dataSize)
        , normalized(copy.normalized)
        , stride(copy.stride)
        , offset(copy.offset)
        , divisor(copy.divisor)
        , name(copy.name)
    {
    }

    GLuint id;
    GLenum type; // like VEC2
    GLint size; // the number of instances of the type, like vec2 x[5] would be 5
    GLenum dataType; // like FLOAT
    GLint dataSize; // num of elements of dataType
    GLboolean normalized;
    GLsizei stride;
    u32 offset;
    u32 divisor;
    std::string name;
};

struct VBOContainer {
    ShaderDataReference ref;
    std::vector<const VBOAttrib*> attrs;
    GLsizei stride;
    GLuint divisor;
    GLuint vbo;
};

struct VAOContainer {
    GLuint vao;
    std::vector<VBOContainer> vbos;
};

struct UniformDefinition {
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
struct UniformContainer {
    UniformContainer(const ShaderDataReference& _ref, u32 _instance)
        : ref(_ref)
        , instance(_instance)
    {
    }

    ShaderDataReference ref;
    u32 instance;
    u32 stride;
    std::vector<const UniformDefinition*> defs;
};

struct UniformHolder {
    std::vector<UniformContainer> containers;
};

struct ShaderData {
    ShaderData(const UniformContainer* unif, u32 dataSize)
    {
        definition.unif = unif;
        data.resize(dataSize);
    }

    ShaderData(const VBOContainer* vbo, u32 dataSize)
    {
        definition.vbo = vbo;
        data.resize(dataSize);
    }

    ~ShaderData()
    {
    }

    union {
        const UniformContainer* unif;
        const VBOContainer* vbo;
    } definition;

    std::vector<char> data;
};

typedef std::unordered_map<ShaderDataReference, ShaderData, ShaderDataReference::Hasher> ShaderDataMap;
typedef std::unordered_map<ShaderDataReference, ShaderData, ShaderDataReference::Hasher>::iterator ShaderDataMapIt;

// GL BATCH
class GL2Batch : public IGraphicBatch {
public:
    GL2Batch(VAOContainer&& vC, const UniformHolder& uC);
    ~GL2Batch();

    // Delete VAO/VBO from gpu.
    void clearVAO();

    // Clear all sectors.
    void clear() override
    {
        totalInstances = 0;
    }

    // Prepare the batch to be rendered
    // Usefull when rendering multiple times the same batch
    void prepare(u32 numInstances) override;

    // Load it to the gpu
    void load() override;

    // Render the loaded data
    void render(Shader* shader) override;

    void setVertexRenderMode(VertexRenderMode mode) override;

protected:
    // Load data into internal buffer
    // This will replace all data from the internal buffer
    void* getShaderData(const ShaderDataReference& ref) override;

private:
    // Members
    GLint renderMode;
    VAOContainer vaoContainer;
    const UniformHolder& uniformContainer;
    u32 totalInstances;
    ShaderDataMap shaderData;
};
}
