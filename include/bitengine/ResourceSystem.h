#pragma once

#include <string>
#include <vector>

#include <map>
#include "System.h"
#include "Texture.h"
#include "SpriteManager.h"


namespace BitEngine{

class ResourceSystem : public System
{
    public:
        typedef std::map<std::string, Texture*> TextureMap;

		ResourceSystem();
		~ResourceSystem();

		bool Init() override;
		void Update() override;
		void Shutdown() override;

        Texture* LoadTexture2D(const std::string& path);

		SpriteManager* getSpriteManager();

    private:
        TextureMap m_textures;

		SpriteManager* m_spriteManager;

};

}
