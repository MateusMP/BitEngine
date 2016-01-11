#pragma once

#include <vector>

#include "System.h"
#include "MessageType.h"

#include "Graphics.h"

#include "EngineConfiguration.h"

// Basic Systems
#include "VideoSystem.h"
#include "EntitySystem.h"
#include "InputSystem.h"
#include "CommandSystem.h"
#include "ResourceSystem.h"
// Entity System Processors
#include "GameLogicProcessor.h"
#include "Transform2DProcessor.h"
#include "Transform3DProcessor.h"
#include "Camera2DProcessor.h"
#include "Camera3DProcessor.h"
#include "Sprite2DHolder.h"
#include "RenderableMeshProcessor.h"

#include "Timer.h"

namespace BitEngine{

class GameEngine
{
    public:
        static void GLFW_ErrorCallback(int error, const char* description);

        GameEngine(const std::string& configFile);
        ~GameEngine();

        bool Run();

        void Message(const WindowClosed& msg);

    protected:
		virtual void CreateSystems() = 0;

        void AddSystem(System *sys);
		System* getSystem(const std::string& name) const;

    private:
        bool InitSystems();
        void ShutdownSystems();

        std::vector<System*> systems;
		std::vector<System*> systemsToShutdown;
        bool running;
		int lastSystemInitialized;

		EngineConfiguration configuration;
};

}
