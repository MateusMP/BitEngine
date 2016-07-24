#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "Common/MathUtils.h"
#include "Core/Logger.h"

#include "DefaultBackends/opengl/GL2/GL2TextureManager.h"

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
		glGenTextures(1, &textureID);
		LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "GENERATING ERROR TEXTURE: " << textureID << error_texture_data.width << error_texture_data.height;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, error_texture_data.width, error_texture_data.height,
			0, GL_RGB, GL_UNSIGNED_BYTE, error_texture_data.pixel_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}

	//

	GL2TextureManager::GL2TextureManager()
	{
		ramInUse = 0;
		gpuMemInUse = 0;
	}

	bool GL2TextureManager::init()
	{
		stbi_set_flip_vertically_on_load(true);

		// Init error texture
		GL2Texture& errorTexture = textures.getResourceAt(0);
		GLuint errorTextureId = GenerateErrorTexture();
		errorTexture.m_textureType = GL_TEXTURE_2D;
		errorTexture.m_textureID = errorTextureId;

		// Erase textures
		for (GL2Texture& t : textures.getResources())
		{
			t.m_textureType = errorTexture.m_textureType;
			t.m_textureID = errorTexture.m_textureID;
			t.m_loaded = false;
		}

		errorTexture.m_loaded = true;

		return true;
	}

	void GL2TextureManager::update()
	{
		bool loadedSomething = false;
		GL2Texture* loadTexture;
		while (rawData.tryPop(loadTexture))
		{
			loadTexture2D(loadTexture->imgData, *loadTexture);

			const uint32 ram = loadTexture->getUsingRamMemory();
			const uint32 gpuMem = loadTexture->getUsingGPUMemory();
			ramInUse += ram;
			gpuMemInUse += gpuMem;

			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << loadTexture->getResourceId() << "  RAM: " << BitEngine::BytesToMB(ram) << " MB - GPU Memory: " << BitEngine::BytesToMB(gpuMem) << " MB";

			loadedSomething = true;
		}

		if (loadedSomething) {
			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "TextureManager MEMORY: RAM: " << BitEngine::BytesToMB(getCurrentRamUsage()) << " MB - GPU Memory: " << BitEngine::BytesToMB(getCurrentGPUMemoryUsage()) << " MB";
		}
	}

	BaseResource* GL2TextureManager::loadResource(ResourceMeta* meta)
	{
		GL2Texture* texture = textures.findResource(meta);

		// Recreate the texture object
		new (texture) GL2Texture(meta);

		if (texture == nullptr)
		{
			uint16 id = textures.addResource(meta);
			texture = &(textures.getResourceAt(id));

			// Make new load request
			loadRawData(loader, meta, this);
		}

		return texture;
	}

	void GL2TextureManager::onResourceLoaded(ResourceLoader::DataRequest& dr)
	{
		GL2Texture* resTexture = textures.findResource(dr.meta);

		// Working on ResourceLoader thread!
		{
			LOG_SCOPE_TIME(BitEngine::EngineLog, "Texture load");

			resTexture->imgData.pixelData = stbi_load_from_memory((unsigned char*)dr.data.data(), dr.data.size(), &resTexture->imgData.width, &resTexture->imgData.height, &resTexture->imgData.color, 0);
			if (resTexture->imgData.pixelData != nullptr) {
				LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << dr.meta->id << " w: " << resTexture->imgData.width << " h: " << resTexture->imgData.height;
			}
			else
			{
				LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << dr.meta->id << " reason: " << stbi_failure_reason();
			}
		}

		waitingData.erase(dr.meta);
		rawData.push(resTexture);
	}

	void GL2TextureManager::onResourceLoadFail(ResourceLoader::DataRequest& dr)
	{
		LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Failed to load " << dr.meta->id;
		waitingData.erase(dr.meta);
	}

	void GL2TextureManager::loadTexture2D(const GL2Texture::StbiImageData& data, GL2Texture& texture)
	{
		GLuint textureID = 0;

		// Texture is not loaded if it's gl id == error texture gl id
		// if (texture.m_textureID != textures.getResourceAt(0).m_textureID)
		if (texture.m_loaded) // TODO: use this instead?
		{
			// Use the same GL index
			textureID = texture.m_textureID;
		}
		else
		{
			// If the texture is not yet loaded, we need to generate a new gl texture
			// Generate new texture index
			glGenTextures(1, &textureID);
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		if (data.color == 1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.pixelData);
			LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "GRAYSCALE TEXTURE!";
		}
		else if (data.color == 2)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RG, GL_UNSIGNED_BYTE, data.pixelData);
			LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "GRAYSCALE ALPHA TEXTURE!";
		}
		else if (data.color == 3) // RGB
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixelData);
		}
		else if (data.color == 4) // RGBA
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.pixelData);
		}
		else {
			LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "LoadTexture: Unknow color " << data.color;
		}

		// MIPMAPS
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Create final texture object
		texture.m_textureID = textureID;
		texture.m_textureType = GL_TEXTURE_2D;
		texture.m_loaded = true;
	}

}