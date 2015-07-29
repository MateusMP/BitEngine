#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "TextureManager.h"
#include "EngineLoggers.h"

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
	printf("GENERATING ERROR TEXTURE: %u %d %d\n", textureID, error_texture_data.width, error_texture_data.height);
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

TextureManager::~TextureManager(){
	for (TextureMap::iterator it = m_textures.begin(); it != m_textures.end(); ++it){
		Texture *tex = it->second;

		glDeleteTextures(1, &tex->m_textureID);
	}

	if (error_texture.m_textureID != 0)
		glDeleteTextures(1, &error_texture.m_textureID);
}

const Texture* TextureManager::LoadTexture2D(const std::string& path)
{
	auto texfound = m_textures.find(path);
	if (texfound != m_textures.end()){
		return texfound->second;
	}

	int w, h, c;
	unsigned char* image = stbi_load(path.c_str(), &w, &h, &c, 0);

	if (image == NULL){
		LOGTO(Error) << "LoadTexture: failed to open texture: " << path << endlog;
		if (error_texture.m_textureID == 0){
			GenerateErrorTexture(error_texture.m_textureID);
		}

		return &error_texture;
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
		LOG() << "LoadTexture: Unknow color " << c << endlog;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);

	// Create final texture object
	Texture *texture = new Texture();
	texture->m_textureID = textureID;
	texture->m_textureType = GL_TEXTURE_2D;

	m_textures[path] = texture;

	return texture;
}

}