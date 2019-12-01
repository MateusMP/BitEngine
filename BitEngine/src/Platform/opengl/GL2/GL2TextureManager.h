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

    public:
        struct StbiImageData {
            StbiImageData()
                : pixelData(nullptr), width(0), height(0), color(0)
            {}
            int width;
            int height;
            int color;
            void* pixelData;
            std::vector<char> fileData;
        };

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

        // Aproximate memory use in ram in bytes
        u32 getUsingRamMemory()
        {
            if (imgData.pixelData) {
                return getUsingGPUMemory() + imgData.fileData.size();
            }

            return imgData.fileData.size();
        }

        // Aproximate memory use in gpu in bytes
        u32 getUsingGPUMemory() {
            return imgData.width*imgData.height * imgData.color;
        }

        size_t releaseMemoryData();

    protected:
        GLuint m_textureID;
        GLuint m_textureType;

        StbiImageData imgData;

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

        BaseResource* loadResource(ResourceMeta* base);

        u32 getCurrentRamUsage() const override {
            return ramInUse;
        }

        u32 getCurrentGPUMemoryUsage() const override {
            return gpuMemInUse;
        }

        void uploadToGPU(GL2Texture* texture);

        static void readJsonProperties(DevResourceLoader* devloader, nlohmann::json& props, ResourceManager* manager, BaseResource* resource) {

        }

        template<typename Serializer>
        static void jsonPropertiesToProd(Serializer*, ResourceManager*, nlohmann::json& props, BaseResource*) {

        }

    private:
        static GLuint GenerateErrorTexture();

        void releaseDriverData(GL2Texture* texture);

        void resourceNotInUse(ResourceMeta* base) override;
        void reloadResource(BaseResource* resource) override;
        void resourceRelease(ResourceMeta* base) override;

        void loadTexture2D(const GL2Texture::StbiImageData& data, GL2Texture& texture);
        void releaseStbiRawData(GL2Texture* texture);
        void scheduleLoadingTasks(ResourceMeta* meta, GL2Texture* texture);

        void resourceRelease(GL2Texture* texture);

        // Members
        TaskManager* taskManager;
        ResourceLoader* loader;
        ResourceIndexer<GL2Texture, 1024> textures;
        ThreadSafeQueue<GL2Texture*> rawData; // raw data loaded and waiting to be sent to gpu
        GL2Texture* errorTexture;

        u32 ramInUse;
        u32 gpuMemInUse;
    };

}
