#include <stdio.h>
#include <stdlib.h>

#include "Core/GameEngine.h"
#include "Core/Logger.h"

#ifdef _DEBUG
	#define LOG_LOGGING_THRESHOLD LOG_ALL

#else
	#define LOG_LOGGING_THRESHOLD ERROR	
#endif

namespace BitEngine{

uint64 Time::ticks = 0;

void GameEngine::GLFW_ErrorCallback(int error, const char* description)
{
    LOG(EngineLog, ERROR) << "GLFW Error: " << error << ": " << description;
}

GameEngine::GameEngine(const std::string& configFile)
	: configuration(configFile)
{
    Channel::AddListener<WindowClosed>(this);
}

GameEngine::~GameEngine()
{

}

void GameEngine::Message(const WindowClosed& msg)
{
    running = false;
}

void GameEngine::AddSystem(System *sys)
{
	if (std::find(systems.begin(), systems.end(), sys) != systems.end())
		return;

    systems.push_back(sys);

	configuration.AddConfiguration(sys->getName(), sys->getConfiguration());
}

System* GameEngine::getSystem(const std::string& name) const
{
	for (System* s : systems){
		if (s->getName().compare(name) == 0){
			return s;
		}
	}

	return nullptr;
}

bool GameEngine::InitSystems()
{
	LOG(EngineLog, WARNING) << "Initializing " << systems.size() << " systems ";

	LOG(EngineLog, WARNING) << "Loadiging system configurations...";
	configuration.LoadConfigurations();

	LOG(EngineLog, WARNING) << "Loadiging systems...";
	lastSystemInitialized = -1;
    for ( System* s : systems )
    {
        if (!s->Init())
        {
            LOG(EngineLog, WARNING) << "System " << s->getName() << " failed to initialize!\n";
            return false;
        }

		++lastSystemInitialized;
		systemsToShutdown.push_back(s);
    }

	LOG(EngineLog, WARNING) << "All systems set!\n";

    return true;
}

void GameEngine::ShutdownSystems()
{
	LOG(EngineLog, WARNING) << "Shutitng down systems...";

	// Shutdown in reverse order
	for (auto it = systemsToShutdown.rbegin(); it != systemsToShutdown.rend(); ++it){
		LOG(EngineLog, WARNING) << "Shutting down " << (*it)->getName();
        (*it)->Shutdown();
		LOG(EngineLog, WARNING) << "done " << (*it)->getName();
		delete *it;
    }

	LOG(EngineLog, WARNING) << "Finalizing... ";

	// Delete systems that were not initialized because of failure
	for (int i = lastSystemInitialized + 1; i < (int)systems.size(); ++i){
		delete systems[i];
	}
	
	LOG(EngineLog, WARNING) << "Systems released!";
}

bool GameEngine::Run()
{
	CreateSystems();

    LOG(EngineLog, WARNING) << "GameEngine::Run";
    glfwSetErrorCallback(GLFW_ErrorCallback);

    running = true;

    if ( InitSystems() )
    {
		Time::ResetTicks();

        while (running)
		{
            for (System *s : systems){
                s->Update();
            }

			Time::Tick();
        }
    }

    ShutdownSystems();

    return running;
}

}
