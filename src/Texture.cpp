#include "Texture.h"

namespace BitEngine{

Texture::Texture()
	: m_textureID(0), m_textureType(0)
{}

GLuint Texture::getID() const{

    return m_textureID;
}

void Texture::bind(GLenum unit) const {
	glActiveTexture(unit);
	glBindTexture(m_textureType, m_textureID);
}

}
