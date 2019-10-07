#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "bitengine/Common/MathUtils.h"
#include "bitengine/Core/Logger.h"
#include "bitengine/Core/TaskManager.h"
#include "bitengine/Core/Assert.h"

#include "Platform/opengl/GL2/GL2TextureManager.h"

namespace BitEngine {

size_t GL2Texture::releaseMemoryData()
{
    std::vector<char> tmp;
    imgData.fileData.swap(tmp);
    return tmp.size();
}

struct ERROR_TEXTURE_DATA {
    int 	 width;
    int 	 height;
    unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char	 pixel_data[32 * 32 * 3 + 1];
};

extern ERROR_TEXTURE_DATA error_texture_data;

GLuint GL2TextureManager::GenerateErrorTexture()
{
    GLuint textureID;
    GL_CHECK(glGenTextures(1, &textureID));
    LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "GENERATING ERROR TEXTURE: " << textureID << error_texture_data.width << error_texture_data.height;
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureID));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, error_texture_data.width, error_texture_data.height,
        0, GL_RGB, GL_UNSIGNED_BYTE, error_texture_data.pixel_data));
    glGenerateMipmap(GL_TEXTURE_2D);
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return textureID;
}

//

class RawTextureLoader : public Task
{
public:
    RawTextureLoader(GL2TextureManager* _manager, GL2Texture* _texture, ResourceLoader::DataRequest& data)
        : Task(Task::TaskMode::NONE, Task::Affinity::BACKGROUND), manager(_manager), texture(_texture), useData(std::move(data))
    {}
    RawTextureLoader(GL2TextureManager* _manager, GL2Texture* _texture, ResourceLoader::RawResourceTask data)
        : Task(Task::TaskMode::NONE, Task::Affinity::BACKGROUND), manager(_manager), texture(_texture), textureData(data), useData(nullptr)
    {}

    // Inherited via Task
    virtual void run() override
    {
        LOG_SCOPE_TIME(BitEngine::EngineLog, "Texture load");

        ResourceLoader::DataRequest dr(nullptr);
        if (textureData)
        {
            dr = std::move(textureData->getData());
        }
        else {
            dr = std::move(useData);
        }

        if (dr.isLoaded())
        {
            texture->imgData.fileData.swap(dr.data);
            texture->imgData.pixelData = stbi_load_from_memory((unsigned char*)texture->imgData.fileData.data(), texture->imgData.fileData.size(), &texture->imgData.width, &texture->imgData.height, &texture->imgData.color, 0);

            if (texture->imgData.pixelData != nullptr) {
                LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << dr.meta->getNameId() << " w: " << texture->imgData.width << " h: " << texture->imgData.height;
                manager->uploadToGPU(texture);
            }
            else
            {
                LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << dr.meta->getNameId() << " reason: " << stbi_failure_reason();
            }
        }
        else
        {
            LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Resource meta " << dr.meta->getNameId() << " on state: " << dr.loadState;
        }
    }

private:
    GL2TextureManager* manager;
    GL2Texture* texture;
    ResourceLoader::DataRequest useData;
    ResourceLoader::RawResourceTask textureData;
};

//

GL2TextureManager::GL2TextureManager(TaskManager* tm)
    : taskManager(tm), loader(nullptr), errorTexture(nullptr)
{
    ramInUse = 0;
    gpuMemInUse = 0;
}

GL2TextureManager::~GL2TextureManager()
{
    for (GL2Texture& t : textures.getResources())
    {
        releaseDriverData(&t);
    }
}

bool GL2TextureManager::init()
{
    stbi_set_flip_vertically_on_load(true);

    ResourceMeta* meta = loader->includeMeta("default", "texture", "TEXTURE");

    // Init error texture
    u16 id = textures.addResource(meta);
    errorTexture = textures.getResourceAddress(id);
    new (errorTexture)GL2Texture(meta); // Reconstruct object giving it the meta

    GLuint errorTextureId = GenerateErrorTexture();
    errorTexture->m_textureType = GL_TEXTURE_2D;
    errorTexture->m_textureID = errorTextureId;

    // Erase textures
    for (GL2Texture& t : textures.getResources())
    {
        t.m_textureType = errorTexture->m_textureType;
        t.m_textureID = errorTexture->m_textureID;
        t.m_loaded = GL2Texture::TextureLoadState::NOT_LOADED;
        t.meta = meta;
    }

    errorTexture->m_loaded = GL2Texture::TextureLoadState::LOADED;

    return true;
}

void GL2TextureManager::shutdown()
{
    for (GL2Texture& texture : textures.getResources()) {
        releaseDriverData(&texture);
    }
}

void GL2TextureManager::releaseDriverData(GL2Texture* texture)
{
    if (texture->m_loaded == GL2Texture::TextureLoadState::LOADED)
    {
        glDeleteTextures(1, &texture->m_textureID);
        texture->m_textureID = errorTexture->m_textureID;
        texture->m_loaded = GL2Texture::TextureLoadState::NOT_LOADED;
        texture->imgData.color = 0;
        texture->imgData.height = 0;
        texture->imgData.width = 0;
    }
}

void GL2TextureManager::releaseStbiRawData(GL2Texture* texture)
{
    GL2Texture::StbiImageData& data = texture->imgData;
    ramInUse -= texture->getUsingRamMemory();
    stbi_image_free(data.pixelData);
    data.pixelData = nullptr;
}

void GL2TextureManager::update()
{
    taskManager->verifyMainThread();
    bool loadedSomething = false;
    GL2Texture* loadTexture;
    while (rawData.tryPop(loadTexture))
    {
        loadTexture2D(loadTexture->imgData, *loadTexture);
        releaseStbiRawData(loadTexture);

        LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << loadTexture->getResourceId() << "  RAM: " << BitEngine::BytesToMB(loadTexture->getUsingRamMemory()) << " MB - GPU Memory: " << BitEngine::BytesToMB(loadTexture->getUsingGPUMemory()) << " MB";

        loadedSomething = true;
    }

    if (loadedSomething) {
        gpuMemInUse = 0;
        ramInUse = 0;
        for (auto t : textures.getResources()) {
            gpuMemInUse += t.getUsingGPUMemory();
            ramInUse += t.getUsingRamMemory();
        }

        LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "TextureManager MEMORY: RAM: " << BitEngine::BytesToMB(getCurrentRamUsage()) << " MB - GPU Memory: " << BitEngine::BytesToMB(getCurrentGPUMemoryUsage()) << " MB";
    }
}

void GL2TextureManager::scheduleLoadingTasks(ResourceMeta* meta, GL2Texture* texture)
{
    texture->m_loaded = GL2Texture::TextureLoadState::LOADING;
    ResourceLoader::RawResourceTask rawDataTask = loader->loadRawData(meta);
    TaskPtr textureLoader = std::make_shared<RawTextureLoader>(this, texture, rawDataTask);
    textureLoader->addDependency(rawDataTask);
    taskManager->addTask(textureLoader);
}

BaseResource* GL2TextureManager::loadResource(ResourceMeta* meta)
{
    GL2Texture* texture = textures.findResource(meta);

    // Recreate the texture object
    if (texture == nullptr)
    {
        u16 id = textures.addResource(meta);
        texture = textures.getResourceAddress(id);

        // Reconstruct in place, giving it the meta
        new (texture)GL2Texture(meta);

        // Make new load request
        scheduleLoadingTasks(meta, texture);
    }
    else
    {
        if (texture->m_loaded == GL2Texture::TextureLoadState::NOT_LOADED)
        {
            if (texture->imgData.fileData.empty())
            {
                scheduleLoadingTasks(meta, texture);
            }
            else
            {
                // Build fake data request result with the data we already have
                ResourceLoader::DataRequest data(meta);
                data.loadState = ResourceLoader::DataRequest::LOADED;
                data.data.swap(texture->imgData.fileData);
                texture->m_loaded = GL2Texture::TextureLoadState::LOADING;

                // Load from the compacted data
                TaskPtr textureLoader = std::make_shared<RawTextureLoader>(this, texture, data);
                taskManager->addTask(textureLoader);
            }
        }
    }

    return texture;
}

void GL2TextureManager::resourceNotInUse(ResourceMeta* meta)
{
    BE_ASSERT(meta != nullptr);
    GL2Texture* texture = textures.findResource(meta);
    BE_ASSERT(texture != nullptr);

    releaseDriverData(texture);
}

void GL2TextureManager::reloadResource(BaseResource* resource)
{
    GL2Texture* texture = static_cast<GL2Texture*>(resource);
    if (texture->m_loaded == GL2Texture::TextureLoadState::LOADING) {
        LOG(EngineLog, BE_LOG_INFO) << "Request to load texture " << resource->getResourceId() << " ignored, already loading...";
        return;
    }
    scheduleLoadingTasks(texture->getMeta(), texture);
}

void GL2TextureManager::resourceRelease(ResourceMeta* meta)
{
    BE_ASSERT(meta != nullptr);
    GL2Texture* texture = textures.findResource(meta);
    BE_ASSERT(texture != nullptr);
    resourceRelease(texture);
}

void GL2TextureManager::resourceRelease(GL2Texture* texture)
{
    this->gpuMemInUse -= texture->getUsingGPUMemory();
    releaseDriverData(texture);
    this->ramInUse -= texture->releaseMemoryData();
}

void GL2TextureManager::uploadToGPU(GL2Texture* texture)
{
    rawData.push(texture);
}

void GL2TextureManager::loadTexture2D(const GL2Texture::StbiImageData& data, GL2Texture& texture)
{
    GLuint textureID = 0;

    if (texture.m_loaded == GL2Texture::TextureLoadState::LOADED)
    {
        // Use the same GL index
        textureID = texture.m_textureID;
    }
    else
    {
        // If the texture is not yet loaded, we need to generate a new gl texture
        GL_CHECK(glGenTextures(1, &textureID));
    }

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureID));
    if (data.color == 1)
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.pixelData));
        LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "GRAYSCALE TEXTURE!";
    }
    else if (data.color == 2)
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RG, GL_UNSIGNED_BYTE, data.pixelData));
        LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "GRAYSCALE ALPHA TEXTURE!";
    }
    else if (data.color == 3) // RGB
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixelData));
    }
    else if (data.color == 4) // RGBA
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.pixelData));
    }
    else {
        LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "LoadTexture: Unknow color " << data.color;
    }

    // MIPMAPS
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    // Create final texture object
    texture.m_textureID = textureID;
    texture.m_textureType = GL_TEXTURE_2D;
    texture.m_loaded = GL2Texture::TextureLoadState::LOADED;
}

}
