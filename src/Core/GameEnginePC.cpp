#include <stdio.h>
#include <stdlib.h>

#include "Core/GameEnginePC.h"
#include "Core/Logger.h"

namespace BitEngine{

GameEnginePC::GameEnginePC(const std::string& configFile, ResourceLoader* _loader)
	: configuration(configFile), loader(_loader)
{
}

GameEnginePC::~GameEnginePC()
{
	delete loader;
}

void GameEnginePC::stopRunning()
{
    running = false;
}

void GameEnginePC::addSystem(System *sys)
{
	auto &it = systems.find(sys->getName());
	if (it == systems.end())
	{
		systems.emplace(sys->getName(), sys);
	}
	else
	{
		LOG(EngineLog, BE_LOG_ERROR) << "System already added: " << sys->getName();
	}
}

System* GameEnginePC::getSystem(const std::string& name)
{
	auto &it = systems.find(name);
	if (it == systems.end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}

bool GameEnginePC::initSystems()
{
	LOG(EngineLog, BE_LOG_INFO) << "Initializing " << systems.size() << " systems ";

	LOG(EngineLog, BE_LOG_INFO) << "Loadiging system configurations...";
	configuration.LoadConfigurations();

	LOG(EngineLog, BE_LOG_INFO) << "Loadiging systems...";
    for ( auto& it : systems )
    {
		System* s = it.second;
        if (!s->Init())
        {
            LOG(EngineLog, BE_LOG_INFO) << "System " << s->getName() << " failed to initialize!\n";
            return false;
        }

		systemsToShutdown.push_back(s);
    }

	LOG(EngineLog, BE_LOG_INFO) << "All systems set!";

    return true;
}

void GameEnginePC::shutdownSystems()
{
	LOG(EngineLog, BE_LOG_INFO) << "Shutitng down systems...";

	// Shutdown in reverse order
	for (auto it = systemsToShutdown.rbegin(); it != systemsToShutdown.rend(); ++it){
		LOG(EngineLog, BE_LOG_INFO) << "Shutting down " << (*it)->getName();
        (*it)->Shutdown();
		LOG(EngineLog, BE_LOG_INFO) << "done " << (*it)->getName();
    }

	LOG(EngineLog, BE_LOG_INFO) << "Clean up memory... ";

	// Delete all systems.
	for (auto& it : systems)
	{
		System* s = it.second;
		delete s;
	}

	systems.clear();
	systemsToShutdown.clear();
	
	LOG(EngineLog, BE_LOG_INFO) << "Systems released!";
}

bool GameEnginePC::run()
{
    LOG(EngineLog, BE_LOG_INFO) << "Run Started";
   
    running = true;
	
    if ( initSystems() )
    {
		Time::ResetTicks();

        while (running)
		{
			messenger.dispatchEnqueued();

			for (auto& it : systems)
			{
				System* s = it.second;
                s->Update();
            }

			Time::Tick();
        }
    }

    shutdownSystems();

    return running;
}

}
