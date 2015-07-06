#include "Texture.h"

namespace BitEngine{


GLuint Texture::getID() const{

    return m_textureID;
}

void Texture::bind(GLenum unit) const {
	glActiveTexture(unit);
	glBindTexture(m_textureType, m_textureID);
}

}
