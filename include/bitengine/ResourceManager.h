#pragma once

#include <string>
#include <map>
#include "Texture.h"

namespace BitEngine{

class ResourceManager
{
    public:
        typedef std::map<std::string, Texture*> TextureMap;

		ResourceManager();
        ~ResourceManager();

        Texture* LoadTexture2D(const std::string& path);

    private:
        TextureMap m_textures;

};


}
