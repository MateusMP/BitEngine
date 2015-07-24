#pragma once

#include <string>
#include <vector>

#include "System.h"
#include "TextureManager.h"
#include "SpriteManager.h"


namespace BitEngine{

class ResourceSystem : public System
{
    public:
		ResourceSystem();
		~ResourceSystem();

		bool Init() override;
		void Update() override;
		void Shutdown() override;
		
		SpriteManager* getSpriteManager();
		TextureManager* getTextureManager();

    private:
		TextureManager* m_textureManager;
		SpriteManager* m_spriteManager;

};

}
