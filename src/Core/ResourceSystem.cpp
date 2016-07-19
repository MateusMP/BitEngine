
#include "Core/ResourceSystem.h"

#include "Core/Graphics.h"
#include "Core/Logger.h"

namespace BitEngine{

ResourceSystem::ResourceSystem(ResourceLoader* resourceLoader)
	: System("Resource"), loader(resourceLoader)
{
}

ResourceSystem::~ResourceSystem(){
}

bool ResourceSystem::Init()
{
	return loader->init();
}

void ResourceSystem::Update()
{
	loader->update();
}

void ResourceSystem::Shutdown()
{
	loader->shutdown();
	delete loader;
}

ResourceLoader* ResourceSystem::getResourceLoader() const {
	return loader;
}

}
