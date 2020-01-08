#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Resources/ResourceManager.h"
#include "BitEngine/Common/ThreadSafeQueue.h"

#include "Platform/opengl/GL2/OpenGL2.h"

namespace BitEngine
{
class DevResourceLoader;

class GL2Texture : public BitEngine::Texture
{
    friend class GL2TextureManager;
    friend class RawTextureLoader;
    friend class TextureUploadToGPU;

public:
    GL2Texture()
        : Texture(nullptr), m_loaded(TextureLoadState::NOT_LOADED)
    {}

    GL2Texture(ResourceMeta* meta)
        : Texture(meta), m_loaded(TextureLoadState::NOT_LOADED)
    {
        m_textureID = 0;
        m_textureType = 0;
    }

    inline u32 getTextureID() const override { return m_textureID; }
    inline u32 getTextureType() const override { return m_textureType; }


protected:
    GLuint m_textureID;
    GLuint m_textureType;

    enum class TextureLoadState {
        NOT_LOADED,
        LOADING,
        LOADED,
    };

    TextureLoadState m_loaded; // true if the texture is in an usable state (loaded in the gpu)

};

class GL2TextureManager : public BitEngine::ResourceManager
{
public:
    GL2TextureManager(TaskManager* taskManager);
    ~GL2TextureManager();

    bool init() override;

    // Load textures that are ready to be sent to the GPU
    void update() override;

    void shutdown() override;

    void setResourceLoader(ResourceLoader* loader) override {
        this->loader = loader;
    }

    BaseResource* loadResource(ResourceMeta* base, PropertyHolder* props) override;

    ptrsize getCurrentRamUsage() const override {
        return ramInUse;
    }

    u32 getCurrentGPUMemoryUsage() const override {
        return gpuMemInUse;
    }

    TaskManager* getTaskManager() {
        return taskManager;
    }

    void addGpuUsage(s32 s) {
        gpuMemInUse += s;
    }
    void addRamUsage(s32 s) {
        ramInUse += s;
    }

    GL2Texture* getErrorTexture() {
        return errorTexture;
    }

private:
    static GLuint GenerateErrorTexture();

    void releaseDriverData(GL2Texture* texture);

    void resourceNotInUse(ResourceMeta* base) override;
    void reloadResource(BaseResource* resource) override;
    void resourceRelease(ResourceMeta* base) override;

    void scheduleLoadingTasks(ResourceMeta* meta, GL2Texture* texture);

    void releaseTexture(GL2Texture* texture);

    // Members
    TaskManager* taskManager;
    ResourceLoader* loader;
    ResourceIndexer<GL2Texture, 1024> textures;
    GL2Texture* errorTexture;

    ptrsize ramInUse;
    u32 gpuMemInUse;
};

}
