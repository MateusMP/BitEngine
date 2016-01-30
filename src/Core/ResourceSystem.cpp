
#include "Core/ResourceSystem.h"

#include "Core/Graphics.h"
#include "Core/Logger.h"

namespace BitEngine{

ResourceSystem::ResourceSystem(ITextureManager* tmng)
	: System("Resource"), m_textureManager(tmng), m_spriteManager(nullptr){

	m_spriteManager = new SpriteManager(m_textureManager);
	m_modelManager = new ModelManager(m_textureManager);

}

ResourceSystem::~ResourceSystem(){
}

bool ResourceSystem::Init()
{
	m_textureManager->Init();
	m_spriteManager->Init();
	m_modelManager->Init();

	return true;
}

void ResourceSystem::Update()
{
	m_textureManager->Update();
}

void ResourceSystem::Shutdown()
{
	delete m_modelManager;
	delete m_spriteManager;
	delete m_textureManager;
}

SpriteManager* ResourceSystem::getSpriteManager() const{
	return m_spriteManager;
}

ITextureManager* ResourceSystem::getTextureManager() const{
	return m_textureManager;
}

ModelManager* ResourceSystem::getModelManager() const{
	return m_modelManager;
}

}
