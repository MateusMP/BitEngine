#pragma once

#include <array>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Common/ThreadSafeQueue.h"

#include "Platform/opengl/GL2/OpenGL2.h"
#include "Platform/opengl/GL2/GL2Shader.h"

namespace BitEngine {

class DevResourceLoader;

class GL2ShaderManager : public ResourceManager
{
public:
    GL2ShaderManager(TaskManager* taskManager);
    ~GL2ShaderManager();

    // Resource Manager
    bool init() override;

    void update() override;

    void shutdown() override;

    void setResourceLoader(ResourceLoader* loader) override {
        this->loader = loader;
    }

    Shader* loadResource(ResourceMeta* base) override;

    void resourceNotInUse(ResourceMeta* meta) override {}
    void reloadResource(BaseResource* resource) override;
    void resourceRelease(ResourceMeta* meta) override {}

    // Shader Manager
    //void Update() override;

    // in bytes
    virtual u32 getCurrentRamUsage() const override {
        return ramInUse;
    }
    virtual u32 getCurrentGPUMemoryUsage() const override {
        return gpuMemInUse;
    }

    void sendToGPU(GL2Shader* shader);

    struct ShaderData {
        const char* vertexData;
        const char* fragmentData;
        // ...
    };

    static void readJsonProperties(DevResourceLoader* devloader, nlohmann::json& props, ResourceManager* manager, BaseResource* resource);

    template<typename Serializer>
    static void jsonPropertiesToProd(Serializer*, ResourceManager*, nlohmann::json& props, BaseResource*) {

    }


private:
    ResourceLoader::RawResourceTask loadShaderSource(DevResourceLoader* loader, const std::string& rpc, GL2Shader* shader);
    void makeFullLoad(ResourceMeta* meta, GL2Shader* shader);

private:
    friend class ShaderSourceLoader;
    ResourceLoader* loader;
    ResourceIndexer<GL2Shader, 32> shaders;
    BitEngine::ThreadSafeQueue<GL2Shader*> resourceLoaded;

    std::unordered_map<ResourceMeta*, GL2Shader*> sourceShaderRelation;

    // Basic shader def
    ShaderDataDefinition basicShaderDef;

    u32 ramInUse;
    u32 gpuMemInUse;

    TaskManager* taskManager;
};

}
