#pragma once

#include <string>
#include <vector>

#include "BitEngine/Core/Graphics/Material.h"

namespace BitEngine {

class ShaderProgram;

class Skeleton
{
public:
};

class Mesh
{
public:
    Mesh(const std::string& name)
        : m_name(name) {}

    virtual ~Mesh() {}

    const std::string& getName() const { return m_name; }

    virtual int getType() const = 0;

    virtual Material* getMaterial() const = 0;

private:
    friend class MeshManager;

    std::string m_name;
};

class Model
{
public:
    Model() {}
    virtual ~Model() {}

    virtual const std::string& getName() const = 0;
    // virtual const Material* getMaterial(int index) const = 0;
    // virtual const Mesh* getMesh(int index) const = 0;
};
}
