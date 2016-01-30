#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "Common/MathUtils.h"
#include "Core/Logger.h"

#include "DefaultBackends/opengl/GlTextureManager.h"

struct ERROR_TEXTURE_DATA {
	int 	 width;
	int 	 height;
	unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
	unsigned char	 pixel_data[32 * 32 * 3 + 1];
};
extern ERROR_TEXTURE_DATA error_texture_data;

GLuint GLTextureManager::GenerateErrorTexture()
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

GLTextureManager::GLTextureManager(BitEngine::IResourceLoader* loader)
	: ITextureManager(), m_loader(loader), m_currentIndex(0)
{
	ramInUse = 0;
	gpuMemInUse = 0;
}

bool GLTextureManager::Init()
{
	stbi_set_flip_vertically_on_load(true);

	// Init error texture
	GLuint errorTexture = GenerateErrorTexture();
	m_textures[0].m_textureType = GL_TEXTURE_2D;
	m_textures[0].m_textureID = errorTexture;
	m_textures[0].m_loaded = true;

	// Erase textures
	for (OpenGLTexture& t : m_textures) 
	{
		t.m_textureType = m_textures[0].m_textureType;
		t.m_textureID = m_textures[0].m_textureID;
		t.m_loaded = false;
	}

	return true;
}

void GLTextureManager::Update()
{
	uint16 localID;

	bool loadedSomething = false;
	while (m_resourceLoaded.tryPop(localID))
	{
		OpenGLTexture& data = m_textures[localID];

		// load texture
		loadTexture2D(data.imgData, data);

		// do not need the file data anymore
		data.clearBaseData();

		m_textures[0].m_textureID = 0;

		// delete pixel data too
		stbi_image_free(data.imgData.pixelData);
		data.imgData.pixelData = nullptr;

		const uint32 ram = data.getUsingRamMemory();
		const uint32 gpuMem = data.getUsingGPUMemory();
		ramInUse += ram;
		gpuMemInUse += gpuMem;

		LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << data.path << "  RAM: " << BitEngine::BytesToMB(ram) << " MB - GPU Memory: " << BitEngine::BytesToMB(gpuMem) << " MB";

		loadedSomething = true;

		break;
	}

	if (loadedSomething) {
		LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "TextureManager MEMORY: RAM: " << BitEngine::BytesToMB(getCurrentRamUsage()) << " MB - GPU Memory: " << BitEngine::BytesToMB(getCurrentGPUMemoryUsage()) << " MB";
	}

}

BitEngine::ITexture* GLTextureManager::getTexture(const std::string& str)
{
	auto it = m_textureByName.find(str);

	if (it == m_textureByName.end())
	{
		uint16 id = getNextIndex();
		m_textureByName.emplace(str, id);

		// Send to load queue
		uint32 request = m_loader->loadRequest(str, &(m_textures[id]), this);
		m_loadRequests[request] = id;

		return &(m_textures[id]);
	}
	else
	{
		return &(m_textures[it->second]);
	}
}

BitEngine::ITexture* GLTextureManager::getTexture(uint32 resourceId)
{
	auto it = m_loadRequests.find(resourceId);

	if (it == m_loadRequests.end())
	{
		return &(m_textures[0]); // TODO: Return error texture?
	}
	else
	{
		if (!m_textures[it->second].m_loaded) 
		{
			m_loader->reloadResource( resourceId, this );
		}

		return &(m_textures[it->second]);
	}
}

void GLTextureManager::onResourceLoaded(uint32 resourceID)
{
	uint16 localID = m_loadRequests[resourceID];
	OpenGLTexture& data = m_textures[localID];

	// Working on ResourceLoader thread!
	{
		LOG_SCOPE_TIME(BitEngine::EngineLog, "Texture load");

		data.imgData.pixelData = stbi_load_from_memory((unsigned char*)data.dataPtr, data.dataSize, &data.imgData.width, &data.imgData.height, &data.imgData.color, 0);
		if (data.imgData.pixelData != nullptr) {
			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << data.path << " w: " << data.imgData.width << " h: " << data.imgData.height;
		}
		else {
			LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << data.path;
		}
	}

	m_resourceLoaded.push(localID);
}

void GLTextureManager::onResourceLoadFail(uint32 resourceID)
{
	LOG(BitEngine::EngineLog, BE_LOG_ERROR ) << "Failed to load " << m_textures[m_loadRequests[resourceID]].path;
}

uint16 GLTextureManager::getNextIndex()
{
	uint16 id;

	if (m_freeIndices.empty())
	{
		id = ++m_currentIndex;
	} 
	else 
	{
		id = m_freeIndices.back();
		m_freeIndices.pop_back();
	}

	return id;
}

void GLTextureManager::loadTexture2D(const OpenGLTexture::StbiImageData& data, OpenGLTexture& texture)
{
	GLuint textureID = 0;
	
	if (data.color == 3) // RGBA
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.pixelData);

		// MIPMAPS
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (data.color == 4)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.pixelData);

		// MIPMAPS
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "LoadTexture: Unknow color " << data.color;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create final texture object
	texture.m_textureID = textureID;
	texture.m_textureType = GL_TEXTURE_2D;
	texture.m_loaded = true;
}