#include <stdio.h>
#include <stdlib.h>

#include "GameEngine.h"
#include "EngineLoggers.h"

#ifdef _DEBUG
	#define MIN_SEVERITY LOG_SEVERITY_VERBOSE

	// to hide logs: LOG_SEVERITY_NO_LOGS
	NEW_CONSOLE_LOG(Verbose, "Verbose",		LOG_SEVERITY_VERBOSE,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Warning, "Warning",		LOG_SEVERITY_WARNING,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Error,	 "Error",		LOG_SEVERITY_ERROR,		MIN_SEVERITY);
	NEW_CONSOLE_LOG(Info,	 "Info",		LOG_SEVERITY_INFO,		MIN_SEVERITY);
#else
	#undef _LOG_SEVERITY_
	#define MIN_SEVERITY LOG_SEVERITY_VERBOSE
//LOG_SEVERITY_ERROR

	// to hide logs: LOG_SEVERITY_NO_LOGS
	// file log
	NEW_LOG(Verbose, "Verbose", "BE_Info.log", LOG_SEVERITY_VERBOSE, MIN_SEVERITY);
	NEW_LOG(Warning, "Warning", "BE_Warning.log", LOG_SEVERITY_WARNING, MIN_SEVERITY);
	NEW_LOG(Error, "Error", "BE_Error.log", LOG_SEVERITY_ERROR, MIN_SEVERITY);
	NEW_LOG(Info, "Info", "BE_Info.log",	LOG_SEVERITY_INFO,	MIN_SEVERITY);

	// console log
	// NEW_CONSOLE_LOG(Verbose, "Verbose", LOG_SEVERITY_VERBOSE, MIN_SEVERITY);
	// NEW_CONSOLE_LOG(Warning, "Warning", LOG_SEVERITY_WARNING, MIN_SEVERITY);
	// NEW_CONSOLE_LOG(Error, "Error", LOG_SEVERITY_ERROR, MIN_SEVERITY);
	// NEW_CONSOLE_LOG(Info, "Info", LOG_SEVERITY_INFO, MIN_SEVERITY);
	
#endif

namespace BitEngine{

uint64 Time::ticks = 0;

void GameEngine::GLFW_ErrorCallback(int error, const char* description)
{
    LOGTO(Error) << "GLFW Error: " << error << ": " << description << endlog;
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
	LOG() << "Initializing " << systems.size() << " systems " << endlog;

	LOG() << "Loadiging system configurations..." << endlog;
	configuration.LoadConfigurations();

	LOG() << "Loadiging systems..." << endlog;
	lastSystemInitialized = -1;
    for ( System* s : systems )
    {
        if (!s->Init())
        {
            LOG() << "System " << s->getName() << " failed to initialize!\n" << endlog;
            return false;
        }

		++lastSystemInitialized;
		systemsToShutdown.push_back(s);
    }

	LOG() << "All systems set!\n" << endlog;

    return true;
}

void GameEngine::ShutdownSystems()
{
	LOG() << "Shutitng down systems..." << endlog;

	// Shutdown in reverse order
	for (auto it = systemsToShutdown.rbegin(); it != systemsToShutdown.rend(); ++it){
		LOG() << "Shutting down " << (*it)->getName() << endlog;
        (*it)->Shutdown();
		LOG() << "done " << (*it)->getName() << endlog;
		delete *it;
    }

	LOG() << "Finalizing... " << endlog;

	// Delete systems that were not initialized because of failure
	for (int i = lastSystemInitialized + 1; i < (int)systems.size(); ++i){
		delete systems[i];
	}
	
	LOG() << "Systems released!" << endlog;
}

bool GameEngine::Run()
{
	CreateSystems();

    LOG() << "GameEngine::Run" << endlog;
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
