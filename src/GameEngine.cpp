#include <stdio.h>
#include <stdlib.h>

#include "Graphics.h"

#include "GameEngine.h"
#include "VideoSystem.h"
#include "InputSystem.h"

#include "EngineLoggers.h"

#ifdef _DEBUG
	#define MIN_SEVERITY LOG_SEVERITY_VERBOSE

	// to hide logs: LOG_SEVERITY_NO_LOGS
	NEW_CONSOLE_LOG(Verbose, "Info",		LOG_SEVERITY_VERBOSE,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Warning, "Warning",		LOG_SEVERITY_WARNING,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Error,	 "Error",		LOG_SEVERITY_ERROR,		MIN_SEVERITY);
#else
	#undef _LOG_SEVERITY_
	#define MIN_SEVERITY LOG_SEVERITY_ERROR

	// to hide logs: LOG_SEVERITY_NO_LOGS
	NEW_LOG(General, "Verbose", "BE_Info.log",	LOG_SEVERITY_INFORMATION, MIN_SEVERITY);
	NEW_LOG(Input,	"Warning",	"BE_Warning.log",	LOG_SEVERITY_INFORMATION, MIN_SEVERITY);
	NEW_LOG(Error,	"Error",	"BE_Error.log",	LOG_SEVERITY_ERROR,		  MIN_SEVERITY);
#endif



namespace BitEngine{

void GameEngine::GLFW_ErrorCallback(int error, const char* description)
{
    LOGTO(Error) << "GLFW Error: " << error << ": " << description << endlog;
}

GameEngine::GameEngine()
{
    Channel::AddListener<WindowClose>(this);

    AddSystem(new VideoSystem());
    AddSystem(new InputSystem());
}

GameEngine::~GameEngine()
{

}

void GameEngine::Message(const WindowClose& msg)
{
    running = false;
}

void GameEngine::AddSystem(System *sys)
{
    systems.push_back(sys);
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
    for ( System* s : systems )
    {
        if (!s->Init())
        {

            LOG() << "System " << s->getName().c_str() << " failed to initialize!\n" << endlog;
            return false;
        }

		systemsToShutdown.push_back(s);
    }

	LOG() << "All systems set!\n" << endlog;

    return true;
}

void GameEngine::ShutdownSystems()
{
	LOG() << "Shutitng down systems..." << endlog;
	for (System* s : systemsToShutdown){
        s->Shutdown();
    }
}

bool GameEngine::Run()
{
    LOG() << "GameEngine::Run" << endlog;
    glfwSetErrorCallback(GLFW_ErrorCallback);

    running = true;

    if ( InitSystems() )
    {
        while (running){
            for (System *s : systems){
                s->Update();
            }
        }
    }

    ShutdownSystems();

    return running;
}

}
