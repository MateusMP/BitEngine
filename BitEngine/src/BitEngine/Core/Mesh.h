#pragma once

#include <string>
#include <vector>

#include "BitEngine/Core/Graphics/Material.h"

namespace BitEngine {

class ShaderProgram;

class Skeleton {
public:
};

class Mesh : public BaseResource {
public:
    enum VertexDataType {
        Vertices = 0,
        TextureCoord,
        Normals,
        Tangent,
    };
    struct DataArray {
        void* data;
        ptrsize size;
    };

    Mesh(ResourceMeta* _meta)
        : BaseResource(_meta)
    {
    }

    virtual ~Mesh() {}

    virtual DataArray getVertexData(VertexDataType type) = 0;
    virtual DataArray getIndicesData(int index) = 0;

    virtual int getType() const = 0;

    virtual Material* getMaterial() const = 0;

private:
    friend class MeshManager;
};

class Model : public BaseResource {
public:
    Model(ResourceMeta* _meta)
        : BaseResource(_meta)
    {
    }

    virtual u32 getMeshCount() = 0;

    virtual Mesh* getMesh(int index) = 0;
};
}
