#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include "OpenGLheaders.h"

#include "Core/ITexture.h"
#include "Core/IResourceManager.h"
#include "Core/ThreadSafeQueue.h"

class OpenGLTexture : public BitEngine::ITexture
{
	friend class GLTextureManager;

	public:
		struct StbiImageData {
			int width;
			int height;
			int color;
			void* pixelData;
		};

		uint32 getTextureID() const override { return m_textureID; }
		uint32 getTextureType() const override { return m_textureType; }

		// Aproximate memory use in ram in bytes
		uint32 getUsingRamMemory() {
			uint32 mem = 0;
			if (dataPtr)
				mem += dataSize;
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

class GLTextureManager : public BitEngine::ITextureManager
{
public:
	GLTextureManager(BitEngine::IResourceLoader* loader);

	bool Init() override;

	// Load textures that are ready to be sent to the GPU
	void Update() override;

	BitEngine::ITexture* getTexture(const std::string& str) override;
	BitEngine::ITexture* getTexture(uint32 id) override;

	uint32 getCurrentRamUsage() override {
		return ramInUse;
	}

	uint32 getCurrentGPUMemoryUsage() override {
		return gpuMemInUse;
	}


private:
	static GLuint GenerateErrorTexture();

	// IResourceManager
	void onResourceLoaded(uint32 resourceID) override;
	void onResourceLoadFail(uint32 resourceID) override;

	//
	uint16 getNextIndex();
	void loadTexture2D(const OpenGLTexture::StbiImageData& data, OpenGLTexture& texture);

	// Members
	uint16 m_currentIndex;
	BitEngine::IResourceLoader* m_loader;
	std::array<OpenGLTexture, 1024> m_textures;
	std::vector<uint16> m_freeIndices;
	std::unordered_map<uint32, uint16> m_loadRequests;
	std::unordered_map<std::string, uint16> m_textureByName;

	BitEngine::ThreadSafeQueue<uint16> m_resourceLoaded;

	uint32 ramInUse;
	uint32 gpuMemInUse;
};