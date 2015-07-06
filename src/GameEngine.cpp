#include <stdio.h>
#include <stdlib.h>

#include "Graphics.h"

#include "GameEngine.h"
#include "VideoSystem.h"
#include "InputSystem.h"

namespace BitEngine{

void GameEngine::GLFW_ErrorCallback(int error, const char* description)
{
    printf("GLFW Error %d: %s\n", error, description);
}

GameEngine::GameEngine()
{
    printf("GameEngine");
    Channel::AddListener<WindowClose>(this);

    AddSystem(new VideoSystem());
    AddSystem(new InputSystem());

    printf("GameEngine();\n");
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

bool GameEngine::InitSystems()
{
    printf("Initializing systems [%d]...\n", systems.size());
    for ( System* s : systems )
    {
        if (!s->Init())
        {
            printf("System %s failed to initialize!\n", s->getName().c_str());
            return false;
        }
    }

    printf("All systems set!\n");

    return true;
}

void GameEngine::ShutdownSystems()
{
    for ( System* s : systems ){
        s->Shutdown();
    }
}

bool GameEngine::Run()
{
    printf("GameEngine::Run");
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
