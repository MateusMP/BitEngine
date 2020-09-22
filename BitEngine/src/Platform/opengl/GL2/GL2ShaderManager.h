#pragma once

#include <array>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Common/ThreadSafeQueue.h"
#include "BitEngine/Core/IO/File.h"

#include "Platform/opengl/GL2/OpenGL2.h"
#include "Platform/opengl/GL2/GL2Shader.h"

namespace BitEngine {

class DevResourceLoader;

struct GL2ShaderInfo {
    ShaderDataDefinition* definition;
    RR<File> vertex;
    RR<File> fragment;
    RR<File> geometry;

    static void read(PropertyHolder* prop, GL2ShaderInfo* obj)
    {
        prop->read("vertex", &obj->vertex);
        prop->read("fragment", &obj->fragment);
        prop->read("geometry", &obj->geometry);
        prop->readObject("definition", obj->definition);
    }
};

class GL2ShaderManager : public ResourceManager {
public:
    GL2ShaderManager(TaskManager* taskManager);
    ~GL2ShaderManager();

    // Resource Manager
    bool init() override;

    void update() override;

    void shutdown() override;

    void setResourceLoader(ResourceLoader* loader) override
    {
        this->loader = loader;
    }

    Shader* loadResource(ResourceMeta* base, PropertyHolder* props) override;

    void resourceNotInUse(ResourceMeta* meta) override {}
    void reloadResource(BaseResource* resource) override;
    void resourceRelease(ResourceMeta* meta) override {}

    // Shader Manager
    //void Update() override;

    // in bytes
    virtual ptrsize getCurrentRamUsage() const override
    {
        return ramInUse;
    }
    virtual u32 getCurrentGPUMemoryUsage() const override
    {
        return gpuMemInUse;
    }

    struct ShaderData {
        const char* vertexData;
        const char* fragmentData;
        // ...
    };

private:
    ResourceLoader::RawResourceTask loadShaderSource(ResourceLoader* loader, RR<File> file, GL2Shader* shader);
    void makeFullLoad(ResourceMeta* meta, GL2Shader* shader);

    void intializeResource(ResourceMeta* meta, GL2Shader* resource);

    void sendToGPU(GL2Shader* shader, const GL2ShaderInfo& info);

private:
    friend class ShaderSourceLoader;
    ResourceLoader* loader;
    ResourceIndexer<GL2Shader, 32> shaders;

    struct ToLoad {
        GL2Shader* shader;
        GL2ShaderInfo info;
    };
    BitEngine::ThreadSafeQueue<ToLoad> resourceLoaded;

    std::unordered_map<ResourceMeta*, GL2Shader*> sourceShaderRelation;

    // Basic shader def
    ShaderDataDefinition basicShaderDef;

    ptrsize ramInUse;
    u32 gpuMemInUse;

    TaskManager* taskManager;
};
}
