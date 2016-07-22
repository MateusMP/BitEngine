#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include "Core/Resources/ResourceManager.h"
#include "Common/ThreadSafeQueue.h"

#include "OpenGL2.h"

namespace BitEngine 
{
	class GL2Texture : public BitEngine::ITexture
	{
		friend class GL2TextureManager;

		public:
		struct StbiImageData {
			StbiImageData()
				: pixelData(nullptr)
			{}
			int width;
			int height;
			int color;
			void* pixelData;
		};

		GL2Texture()
			: ITexture(nullptr)
		{}

		GL2Texture(ResourceMeta* meta) 
			: ITexture(meta)
		{
			m_textureID = 0;
			m_textureType = 0;
		}

		inline uint32 getTextureID() const override { return m_textureID; }
		inline uint32 getTextureType() const override { return m_textureType; }

		// Aproximate memory use in ram in bytes
		uint32 getUsingRamMemory()
		{
			if (imgData.pixelData) {
				return getUsingGPUMemory();
			}

			return 0;
		}

		// Aproximate memory use in gpu in bytes
		uint32 getUsingGPUMemory() {
			return imgData.width*imgData.height * imgData.color;
		}

		protected:
		GLuint m_textureID;
		GLuint m_textureType;

		StbiImageData imgData;

		bool m_loaded; // true if the texture is in an usable state (loaded in the gpu)
	};

	class GL2TextureManager : public BitEngine::ResourceManager
	{
		public:
		GL2TextureManager();

		bool init() override;

		// Load textures that are ready to be sent to the GPU
		void update() override;

		void setResourceLoader(ResourceLoader* loader) override {
			this->loader = loader;
		}

		BaseResource* loadResource(ResourceMeta* base);
		
		uint32 getCurrentRamUsage() const override {
			return ramInUse;
		}

		uint32 getCurrentGPUMemoryUsage() const override {
			return gpuMemInUse;
		}


		private:
			static GLuint GenerateErrorTexture();

			// IResourceManager
			void onResourceLoaded(ResourceLoader::DataRequest& dr) override;
			void onResourceLoadFail(ResourceLoader::DataRequest& dr) override;
		
			void loadTextureRaw(ResourceLoader::DataRequest& dr);

			void loadTexture2D(const GL2Texture::StbiImageData& data, GL2Texture& texture);

			// Members
			ResourceLoader* loader;
			ResourceIndexer<GL2Texture, 1024> textures;
			ThreadSafeQueue<GL2Texture*> rawData; // raw data loaded and waiting to be sent to gpu
			std::unordered_map<uint32, uint16> loadRequests;
				
			uint32 ramInUse;
			uint32 gpuMemInUse;
	};

}