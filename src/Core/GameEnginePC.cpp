#include <stdio.h>
#include <stdlib.h>

#include "Core/GameEnginePC.h"
#include "Core/Logger.h"

namespace BitEngine{

uint64 Time::ticks = 0;

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
	if (std::find(systems.begin(), systems.end(), sys) != systems.end())
		return;

    systems.push_back(sys);
}

System* GameEnginePC::getSystem(const std::string& name)
{
	for (System* s : systems){
		
		if (name.compare(s->getName()) == 0){
			return s;
		}
	}

	return nullptr;
}

bool GameEnginePC::initSystems()
{
	LOG(EngineLog, BE_LOG_INFO) << "Initializing " << systems.size() << " systems ";

	LOG(EngineLog, BE_LOG_INFO) << "Loadiging system configurations...";
	configuration.LoadConfigurations();

	LOG(EngineLog, BE_LOG_INFO) << "Loadiging systems...";
	lastSystemInitialized = -1;
    for ( System* s : systems )
    {
        if (!s->Init())
        {
            LOG(EngineLog, BE_LOG_INFO) << "System " << s->getName() << " failed to initialize!\n";
            return false;
        }

		++lastSystemInitialized;
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
		delete *it;
    }

	LOG(EngineLog, BE_LOG_INFO) << "Finalizing... ";

	// Delete systems that were not initialized because of failure
	for (int i = lastSystemInitialized + 1; i < (int)systems.size(); ++i){
		delete systems[i];
	}
	
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
			messenger.Update();

            for (System *s : systems){
                s->Update();
            }

			Time::Tick();
        }
    }

    shutdownSystems();

    return running;
}

}
