#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include "Core/System.h"
#include "Core/SpriteManager.h"
#include "Core/ModelManager.h"
#include "Core/IResourceManager.h"

namespace BitEngine{

// EXAMPLE

class ResourceSystem : public System
{
    public:
		ResourceSystem(IResourceLoader* loader, ITextureManager* tmng);
		~ResourceSystem();

		bool Init() override;
		void Update() override;
		void Shutdown() override;
		
		SpriteManager* getSpriteManager() const;
		ITextureManager* getTextureManager() const;
		ModelManager* getModelManager() const;

    private:
		IResourceLoader* m_resourceLoader;
		ITextureManager* m_textureManager;
		SpriteManager* m_spriteManager;
		ModelManager* m_modelManager;
};

}
