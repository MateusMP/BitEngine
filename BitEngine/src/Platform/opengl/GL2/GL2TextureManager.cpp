#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "BitEngine/Common/MathUtils.h"
#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Assert.h"

#include "Platform/opengl/GL2/GL2TextureManager.h"

namespace BitEngine {

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
struct StbiImageData {
    StbiImageData()
        : pixelData(nullptr), width(0), height(0), color(0)
    {}
    int width;
    int height;
    int color;
    void* pixelData;
};

void releaseStbiData(StbiImageData& data) {
    stbi_image_free(data.pixelData);
    data.pixelData = nullptr;
}

class TextureUploadToGPU : public Task {
public:
    TextureUploadToGPU(GL2TextureManager* tm, GL2Texture* tex, StbiImageData data)
        : Task(Task::TaskMode::REPEATING, Task::Affinity::MAIN),
        state(UploadState::CREATE_BUFFERS), textureManager(tm), texture(tex), pbo(0), storage(0), imageData(data)
    {
        textureID = tex->m_textureID;
    }

    void run() override {
        BE_PROFILE_FUNCTION();
        const u32 size = imageData.width*imageData.height*imageData.color;
        switch (state) {
        case UploadState::CREATE_BUFFERS: // On Main thread
        {
            BE_PROFILE_SCOPE("UploadState::CREATE_BUFFERS");
            if (texture->m_textureID == textureManager->getErrorTexture()->m_textureID) {
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
                GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, imageData.width, imageData.height));
                GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
            }
            glGenBuffers(1, &pbo);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
            GL_CHECK(glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW));
            GL_CHECK(storage = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            state = UploadState::COPYING_DATA;
            setAffinity(Task::Affinity::BACKGROUND); // Next time will be executed as a background task
            textureManager->addRamUsage(size);
        }
            break;

        case UploadState::COPYING_DATA: 
        {
            BE_PROFILE_SCOPE("UploadState::COPYING_DATA");
            // Copy data to buffer in background
            std::memcpy(storage, imageData.pixelData, size);
            state = UploadState::FINISHING;
            releaseStbiData(imageData);
            setAffinity(Task::Affinity::MAIN);
        }
            break;
        case UploadState::FINISHING: // ON Main thread
        {
            BE_PROFILE_SCOPE("UploadState::FINISHING");
            textureManager->addRamUsage(-(s32)size); // We wait until we're on main thread to avoid concurrency issues
            bindTextureDataUsingPBO();
            stopRepeating();
            textureManager->addGpuUsage(size); // TODO: Reduce gpu usage on unload.
        }
            break;
        }
    }

private:
    GLenum stbiColorToGLEnum(int color) {
        switch (color) {
        case 1:   return GL_RED;
        case 2:   return GL_RG;
        case 3:   return GL_RGB;
        case 4:   return GL_RGBA;
        }
        BE_INVALID_PATH("Bad color value");
    }

    void syncLoadTexture2D(const StbiImageData& data, GL2Texture& texture)
    {
        BE_PROFILE_FUNCTION();

        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture.m_textureID));
        if (data.color == 1)
        {
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.pixelData));
        }
        else if (data.color == 2)
        {
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RG, GL_UNSIGNED_BYTE, data.pixelData));
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

        // Setup texture obj
        texture.m_textureType = GL_TEXTURE_2D;
        texture.m_loaded = GL2Texture::TextureLoadState::LOADED;
    }

    void bindTextureDataUsingPBO()
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo));
        GL_CHECK(glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER));

        if (imageData.color == 1)
        {
            GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.width, imageData.height, GL_RED, GL_UNSIGNED_BYTE, NULL));
        }
        else if (imageData.color == 2)
        {
            GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.width, imageData.height, GL_RG, GL_UNSIGNED_BYTE, NULL));
        }
        else if (imageData.color == 3) // RGB
        {
            GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.width, imageData.height, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        }
        else if (imageData.color == 4) // RGBA
        {
            GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imageData.width, imageData.height, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
        }
        else {
            LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "LoadTexture: Unknow color " << imageData.color;
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        texture->m_textureType = GL_TEXTURE_2D;
        texture->m_loaded = GL2Texture::TextureLoadState::LOADED;
        texture->m_textureID = textureID; // We might have created the id or used the same depending on the texture state
    }


private:
    enum class UploadState {
        CREATE_BUFFERS,
        COPYING_DATA,
        FINISHING,
    };
    UploadState state;
    GL2TextureManager* textureManager;
    GL2Texture* texture;
    GLuint pbo;
    GLubyte* storage;
    StbiImageData imageData;
    GLuint textureID;

};

// TODO: Possibly merge this with above task as a new state?
class RawTextureLoader : public Task
{
public:
    RawTextureLoader(GL2TextureManager* _manager, GL2Texture* _texture, ResourceLoader::RawResourceTask data)
        : Task(Task::TaskMode::NONE, Task::Affinity::BACKGROUND), manager(_manager), texture(_texture), textureData(data)
    {}

    // Inherited via Task
    virtual void run() override
    {

        ResourceLoader::DataRequest& dr = textureData->getData();
        if (dr.isLoaded())
        {
            StbiImageData imgData;
            {
                BE_PROFILE_SCOPE("stbi_load");
                imgData.pixelData = stbi_load_from_memory((unsigned char*)dr.data, dr.size, &imgData.width, &imgData.height, &imgData.color, 0);
            }

            if (imgData.pixelData != nullptr) {
                LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << texture->getMeta()->getNameId() << " w: " << imgData.width << " h: " << imgData.height;
                manager->getTaskManager()->addTask(std::make_shared<TextureUploadToGPU>(manager, texture, imgData));
            }
            else
            {
                LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << texture->getMeta()->getNameId() << " reason: " << stbi_failure_reason();
            }
        }
        else
        {
            LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Resource meta " << texture->getMeta()->getNameId() << " on state: " << dr.loadState;
        }
    }

private:
    GL2TextureManager* manager;
    GL2Texture* texture;
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

static ResourceMeta errorTextureMeta;

bool GL2TextureManager::init()
{
    BE_PROFILE_FUNCTION();
    stbi_set_flip_vertically_on_load(true);

    ResourceMeta* meta = &errorTextureMeta;

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
    }
}

void GL2TextureManager::update()
{
    // Do nothing
}

void GL2TextureManager::scheduleLoadingTasks(ResourceMeta* meta, GL2Texture* texture)
{
    BE_PROFILE_FUNCTION();
    texture->m_loaded = GL2Texture::TextureLoadState::LOADING;
    ResourceLoader::RawResourceTask rawDataTask = loader->requestResourceData(meta);
    TaskPtr textureLoader = std::make_shared<RawTextureLoader>(this, texture, rawDataTask);
    textureLoader->addDependency(rawDataTask);
    taskManager->addTask(textureLoader);
}

BaseResource* GL2TextureManager::loadResource(ResourceMeta* meta, PropertyHolder* props)
{
    BE_PROFILE_FUNCTION();
    GL2Texture* texture = textures.findResource(meta);

    // Recreate the texture object
    if (texture == nullptr)
    {
        u16 id = textures.addResource(meta);
        texture = textures.getResourceAddress(id);

        // Reconstruct in place, giving it the meta
        new (texture)GL2Texture(meta);
        texture->m_textureID = errorTexture->m_textureID;

        // Make new load request
        scheduleLoadingTasks(meta, texture);
    }
    else
    {
        if (texture->m_loaded == GL2Texture::TextureLoadState::NOT_LOADED)
        {
            scheduleLoadingTasks(meta, texture);
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
    releaseTexture(texture);
}

void GL2TextureManager::releaseTexture(GL2Texture* texture)
{
    releaseDriverData(texture);
}


}
