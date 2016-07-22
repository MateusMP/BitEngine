
#include "Core/ResourceSystem.h"

#include "Core/Graphics.h"
#include "Core/Logger.h"

namespace BitEngine{

	ResourceSystem::ResourceSystem(GameEngine* ge)
		: System(ge)
	{
	}

	ResourceSystem::~ResourceSystem(){
	}

	bool ResourceSystem::Init()
	{
		loader = getEngine()->getResourceLoader();
		return loader->init();
	}

	void ResourceSystem::Update()
	{
		loader->update();
	}

	void ResourceSystem::Shutdown()
	{
		loader->shutdown();
	}

	ResourceLoader* ResourceSystem::getResourceLoader() const {
		return loader;
	}

}
