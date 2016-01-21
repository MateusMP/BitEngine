#pragma once

#include <vector>

#include "Core/System.h"
#include "Core/MessageType.h"
#include "Core/Graphics.h"
#include "Core/Timer.h"
#include "Core/EngineConfiguration.h"

// Basic Systems
#include "Core/VideoSystem.h"
#include "Core/InputSystem.h"
#include "Core/CommandSystem.h"
#include "Core/ResourceSystem.h"
#include "Core/ECS/EntitySystem.h"
// Entity System Processors
#include "Core/ECS/GameLogicProcessor.h"
#include "Core/ECS/Transform2DProcessor.h"
#include "Core/ECS/Transform3DProcessor.h"
#include "Core/ECS/Camera2DProcessor.h"
#include "Core/ECS/Camera3DProcessor.h"
#include "Core/ECS/Sprite2DHolder.h"
#include "Core/ECS/RenderableMeshProcessor.h"


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
