#include <stdio.h>
#include <stdlib.h>

#include "Graphics.h"

#include "GameEngine.h"
#include "VideoSystem.h"
#include "InputSystem.h"
#include "EntitySystem.h"
#include "CommandSystem.h"

// Entity System Processors
#include "GameLogicProcessor.h"
#include "Transform2DProcessor.h"
#include "Camera2DProcessor.h"
#include "Sprite2DProcessor.h"

#include "EngineLoggers.h"

#ifdef _DEBUG
	#define MIN_SEVERITY LOG_SEVERITY_VERBOSE

	// to hide logs: LOG_SEVERITY_NO_LOGS
	NEW_CONSOLE_LOG(Verbose, "Info",		LOG_SEVERITY_VERBOSE,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Warning, "Warning",		LOG_SEVERITY_WARNING,	MIN_SEVERITY);
	NEW_CONSOLE_LOG(Error,	 "Error",		LOG_SEVERITY_ERROR,		MIN_SEVERITY);
#else
	#undef _LOG_SEVERITY_
	#define MIN_SEVERITY LOG_SEVERITY_VERBOSE
//LOG_SEVERITY_ERROR

	// to hide logs: LOG_SEVERITY_NO_LOGS
	// file log
	NEW_LOG(Verbose, "Verbose", "BE_Info.log", LOG_SEVERITY_VERBOSE, MIN_SEVERITY);
	NEW_LOG(Warning, "Warning", "BE_Warning.log", LOG_SEVERITY_WARNING, MIN_SEVERITY);
	NEW_LOG(Error, "Error", "BE_Error.log", LOG_SEVERITY_ERROR, MIN_SEVERITY);

	// console log
	// NEW_CONSOLE_LOG(Verbose, "Verbose", LOG_SEVERITY_VERBOSE, MIN_SEVERITY);
	// NEW_CONSOLE_LOG(Warning, "Warning", LOG_SEVERITY_WARNING, MIN_SEVERITY);
	// NEW_CONSOLE_LOG(Error, "Error", LOG_SEVERITY_ERROR, MIN_SEVERITY);
	
#endif

#define PRIORITY_ESP_GAMELOGIC		0
#define PRIORITY_ESP_TRANSFORM2D	1
#define PRIORITY_ESP_CAMERA2D		5
#define PRIORITY_ESP_SPR2DRENDER	10


namespace BitEngine{

void GameEngine::GLFW_ErrorCallback(int error, const char* description)
{
    LOGTO(Error) << "GLFW Error: " << error << ": " << description << endlog;
}

GameEngine::GameEngine()
{
    Channel::AddListener<WindowClose>(this);

	ResourceSystem* rsrc = new ResourceSystem();

	EntitySystem* es = new EntitySystem(rsrc, nullptr);

    AddSystem(new InputSystem());
	AddSystem(new CommandSystem());
    AddSystem(new VideoSystem());
	AddSystem(rsrc);
	AddSystem( es );

	// Create entity system processors:
	Transform2DProcessor *t2p = new Transform2DProcessor();
	Camera2DProcessor *c2p = new Camera2DProcessor(es, t2p);
	Sprite2DProcessor *spr2d = new Sprite2DProcessor(es, t2p, c2p);
	GameLogicProcessor *glp = new GameLogicProcessor(es);

	es->RegisterComponentHolderProcessor<GameLogicComponent>(glp, PRIORITY_ESP_GAMELOGIC, UpdateEvent::ALL);
	es->RegisterComponentHolderProcessor<Transform2DComponent>(t2p, PRIORITY_ESP_TRANSFORM2D, UpdateEvent::EndFrame);
	es->RegisterComponentHolderProcessor<Camera2DComponent>(c2p, PRIORITY_ESP_CAMERA2D, UpdateEvent::EndFrame);
	es->RegisterComponentHolderProcessor<Sprite2DComponent>(spr2d, PRIORITY_ESP_SPR2DRENDER, UpdateEvent::EndFrame);
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

	// Shutdown in reverse order
	for (auto it = systemsToShutdown.rbegin(); it != systemsToShutdown.rend(); ++it){
        (*it)->Shutdown();
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
