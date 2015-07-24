#pragma once

#include "Graphics.h"

namespace BitEngine
{

class Texture
{
    public:
		friend class TextureManager;

        GLuint getID() const;

		void bind(GLenum unit) const;

    private:
        GLenum m_textureType; ///< OpenGL texture type (GL_TEXTURE_??? , )
        GLuint m_textureID;

};


}
