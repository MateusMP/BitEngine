#include "bitengine/Core/ResourceSystem.h"

#include "bitengine/Core/Logger.h"

namespace BitEngine{

	ResourceSystem::ResourceSystem(GameEngine* ge)
		: System(ge)
	{
		loader = getEngine()->getResourceLoader();
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
	}

	ResourceLoader* ResourceSystem::getResourceLoader() const {
		return loader;
	}

}