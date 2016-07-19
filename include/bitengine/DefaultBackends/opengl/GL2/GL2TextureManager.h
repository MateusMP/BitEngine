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
			int width;
			int height;
			int color;
			void* pixelData;
		};

		inline uint32 getTextureID() const override { return m_textureID; }
		inline uint32 getTextureType() const override { return m_textureType; }

		// Aproximate memory use in ram in bytes
		uint32 getUsingRamMemory()
		{
			uint32 mem = data.size();

			if (imgData.pixelData)
				mem += getUsingGPUMemory();

			return mem;
		}

		// Aproximate memory use in gpu in bytes
		uint32 getUsingGPUMemory() {
			return imgData.width*imgData.height * 3; // considers 3 colors only
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
		void onResourceLoaded(uint32 resourceID) override;
		void onResourceLoadFail(uint32 resourceID) override;
		
		void loadTexture2D(const GL2Texture::StbiImageData& data, GL2Texture& texture);

		// Members
		ResourceLoader* loader;
		ResourceIndexer<GL2Texture, 1024> textures;
		BitEngine::ThreadSafeQueue<uint16> resourceLoaded;
		std::unordered_map<uint32, uint16> loadRequests;
		
		uint32 ramInUse;
		uint32 gpuMemInUse;
	};

}