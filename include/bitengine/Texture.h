#pragma once

#include "Graphics.h"

namespace BitEngine
{

class Texture
{
    public:
		Texture();

		// Destructor is PRIVATE.

        GLuint getID() const;

		void bind(GLenum unit) const;

    private:
		friend class TextureManager;
		~Texture(){} // Only the texture manager may delete Textures.

        GLenum m_textureType; ///< OpenGL texture type (GL_TEXTURE_??? , )
        GLuint m_textureID;
};


}
