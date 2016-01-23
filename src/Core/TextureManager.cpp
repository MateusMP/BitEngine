#define STB_IMAGE_IMPLEMENTATION
#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <stb_image.h>

#include "Core/TextureManager.h"
#include "Core/Logger.h"

namespace BitEngine{


struct ERROR_TEXTURE_DATA {
	int 	 width;
	int 	 height;
	unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
	unsigned char	 pixel_data[32 * 32 * 3 + 1];
};
extern ERROR_TEXTURE_DATA error_texture_data;
static void GenerateErrorTexture(GLuint& textureID){
	glGenTextures(1, &textureID);
	LOG(EngineLog, BE_LOG_VERBOSE) << "GENERATING ERROR TEXTURE: " << textureID << error_texture_data.width << error_texture_data.height;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, error_texture_data.width, error_texture_data.height,
		0, GL_RGB, GL_UNSIGNED_BYTE, error_texture_data.pixel_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

TextureManager::TextureManager()
{
	stbi_set_flip_vertically_on_load(true);

	error_texture.m_textureType = GL_TEXTURE_2D;
	error_texture.m_textureID = 0;
}

TextureManager::~TextureManager()
{
	for (TextureMap::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		if (it->second.ready)
		{
			Texture *tex = it->second.data;
			releasetexture(tex);
		}
	}

	if (error_texture.m_textureID != 0)
		glDeleteTextures(1, &error_texture.m_textureID);
}

void TextureManager::releasetexture(Texture* tex)
{
	glDeleteTextures(1, &tex->m_textureID);
	delete tex;
}

bool TextureManager::Init()
{
	GenerateErrorTexture(error_texture.m_textureID);

	return true;
}

const Texture* TextureManager::getErrorTexture()
{
	return &error_texture;
}

const Texture* TextureManager::getTexture(const std::string& name)
{
	const auto& it = m_textures.find(name);
	if (it != m_textures.end())
	{
		// Wait something is loading it?
		while (it->second.ready == false){}

		return it->second.data;
	}
	else
	{
		const Texture* t = doLoad(name);
		if (t != nullptr){
			return t;
		}
	}

	return getErrorTexture();
}

void TextureManager::LoadPackage(const DataPackage* package)
{
	for (const auto& it : package->getItems())
	{
		// Now load
		doLoad(it.first);
	}

	m_LoadedPackages.emplace(package);
}

void TextureManager::ReleasePackage(const DataPackage* package)
{
	for (const auto& it : package->getItems())
	{
		const std::string& path = it.first;
		auto texfound = m_textures.find(path);

		// If resource is already loaded
		// ref count it
		if (texfound != m_textures.end())
		{
			if (--(texfound->second.countUsing) == 0)
			{
				releasetexture(texfound->second.data);
				m_textures.erase(texfound);
			}
		}
	}

	m_LoadedPackages.erase(package);
}

const Texture* TextureManager::doLoad(const std::string& path)
{
	auto texfound = m_textures.find(path);

	// If resource is already loaded
	// ref count it
	if (texfound != m_textures.end() && texfound->second.ready)
	{
		texfound->second.countUsing++;
		return texfound->second.data;
	}

	// Create a not ready reference
	auto alocated = m_textures.emplace(path, nullptr);

	// Load
	Texture* texture = loadTexture2D(path);

	// Save
	if (texture != nullptr){
		alocated.first->second.data = texture;
		alocated.first->second.ready = true;
	}

	return texture;
}

Texture* TextureManager::loadTexture2D(const std::string& path)
{
	int w, h, c;
	unsigned char* image = stbi_load(path.c_str(), &w, &h, &c, 0);

	if (image == NULL){
		LOG(EngineLog, BE_LOG_ERROR) << "LoadTexture: failed to open texture: " << path << "\n" << stbi_failure_reason();
		return nullptr;
	}

	GLuint textureID;
	if (c == 3) // RGBA
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

		// MIPMAPS
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (c == 4)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

		// MIPMAPS
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else{
		LOG(EngineLog, BE_LOG_WARNING) << "LoadTexture: Unknow color " << c;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);

	// Create final texture object
	Texture *texture = new Texture();
	texture->m_textureID = textureID;
	texture->m_textureType = GL_TEXTURE_2D;

	return texture;
}

}