
#include "ResourceSystem.h"

#include "Graphics.h"
#include "Logger.h"

namespace BitEngine{

ResourceSystem::ResourceSystem()
	: System("Resource"), m_textureManager(nullptr), m_spriteManager(nullptr){
}

ResourceSystem::~ResourceSystem(){
}

bool ResourceSystem::Init()
{
	m_textureManager = new TextureManager();
	m_spriteManager = new SpriteManager();

	return true;
}

void ResourceSystem::Update()
{

}

void ResourceSystem::Shutdown()
{
	delete m_spriteManager;
	delete m_textureManager;
}

SpriteManager* ResourceSystem::getSpriteManager() const{
	return m_spriteManager;
}

TextureManager* ResourceSystem::getTextureManager() const{
	return m_textureManager;
}

}
