#pragma once

#include <string>
#include <vector>

#include "Core/System.h"
#include "Core/TextureManager.h"
#include "Core/SpriteManager.h"
#include "Core/ModelManager.h"

namespace BitEngine{

class ResourceSystem : public System
{
    public:
		ResourceSystem();
		~ResourceSystem();

		bool Init() override;
		void Update() override;
		void Shutdown() override;
		
		SpriteManager* getSpriteManager() const;
		TextureManager* getTextureManager() const;
		ModelManager* getModelManager() const;

    private:
		TextureManager* m_textureManager;
		SpriteManager* m_spriteManager;
		ModelManager* m_modelManager;

};

}
