#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "Common/MathUtils.h"
#include "Core/Logger.h"
#include "Core/TaskManager.h"

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

	class RawTextureLoader : public Task
	{
		public:
		RawTextureLoader(GL2TextureManager* _manager, GL2Texture* _texture, ResourceLoader::RawResourceTask data)
			: Task(Task::TaskMode::NONE, Task::Affinity::BACKGROUND), manager(_manager), texture(_texture), textureData(data)
		{}

		// Inherited via Task
		virtual void run() override
		{
			LOG_SCOPE_TIME(BitEngine::EngineLog, "Texture load");

			ResourceLoader::DataRequest& dr = textureData->getData();
			
			if (dr.isLoaded())
			{
				texture->imgData.pixelData = stbi_load_from_memory((unsigned char*)dr.data.data(), dr.data.size(), &texture->imgData.width, &texture->imgData.height, &texture->imgData.color, 0);
				if (texture->imgData.pixelData != nullptr) {
					LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << dr.meta->id << " w: " << texture->imgData.width << " h: " << texture->imgData.height;
					manager->uploadToGPU(texture);
				}
				else
				{
					LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << dr.meta->id << " reason: " << stbi_failure_reason();
				}
			}
			else
			{
				LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Resource meta " << dr.meta->id << " on state: " << dr.loadState;
			}
		}

		private:
			GL2TextureManager* manager;
			GL2Texture* texture;
			ResourceLoader::RawResourceTask textureData;
	};

	//

	GL2TextureManager::GL2TextureManager()
	{
		ramInUse = 0;
		gpuMemInUse = 0;
	}

	bool GL2TextureManager::init()
	{
		stbi_set_flip_vertically_on_load(true);

		ResourceMeta* meta = loader->includeMeta("default", "texture", "TEXTURE");

		// Init error texture
		u16 id = textures.addResource(meta);
		GL2Texture& errorTexture = textures.getResourceAt(id);
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

			const u32 ram = loadTexture->getUsingRamMemory();
			const u32 gpuMem = loadTexture->getUsingGPUMemory();
			ramInUse += ram;
			gpuMemInUse += gpuMem;

			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << loadTexture->getResourceId() << "  RAM: " << BitEngine::BytesToMB(ram) << " MB - GPU Memory: " << BitEngine::BytesToMB(gpuMem) << " MB";

			loadedSomething = true;

			finishedLoading(loadTexture->getMeta());
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
			u16 id = textures.addResource(meta);
			texture = &(textures.getResourceAt(id));

			// Make new load request
			ResourceLoader::RawResourceTask rawDataTask = loadRawData(loader, meta);

			TaskPtr textureLoader = std::make_shared<RawTextureLoader>(this, texture, rawDataTask);
			textureLoader->addDependency(rawDataTask);

			loader->getEngine()->getTaskManager()->addTask(textureLoader);
		}

		return texture;
	}

	void GL2TextureManager::uploadToGPU(GL2Texture* texture)
	{
		rawData.push(texture);
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