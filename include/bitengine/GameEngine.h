#pragma once

#include <vector>

#include "System.h"
#include "MessageType.h"

#include "Graphics.h"

// Basic Systems
#include "VideoSystem.h"
#include "EntitySystem.h"
#include "InputSystem.h"
#include "CommandSystem.h"
// Entity System Processors
#include "GameLogicProcessor.h"
#include "Transform2DProcessor.h"
#include "Transform3DProcessor.h"
#include "Camera2DProcessor.h"
#include "Camera3DProcessor.h"
#include "Sprite2DProcessor.h"
#include "RenderableMeshProcessor.h"


namespace BitEngine{

class GameEngine
{
    public:
        static void GLFW_ErrorCallback(int error, const char* description);

        GameEngine();
        ~GameEngine();

        bool Run();

        void Message(const WindowClose& msg);

    protected:
		virtual void CreateSystems()
		{
			printf("Default systems...\n");

			const uint32 PRIORITY_ESP_GAMELOGIC		= 0;
			const uint32 PRIORITY_ESP_TRANSFORM2D	= 1;
			const uint32 PRIORITY_ESP_CAMERA2D		= 5;
			const uint32 PRIORITY_ESP_SPR2DRENDER	= 10;

			ResourceSystem* rsrc = new ResourceSystem();

			EntitySystem* es = new EntitySystem(rsrc);

			AddSystem(new InputSystem());
			AddSystem(new CommandSystem());
			AddSystem(new VideoSystem());
			AddSystem(rsrc);
			AddSystem(es);

			// Create entity system processors:
			Transform2DProcessor *t2p = new Transform2DProcessor();
			Camera2DProcessor *c2p = new Camera2DProcessor(es, t2p);
			Sprite2DProcessor *spr2d = new Sprite2DProcessor();
			GameLogicProcessor *glp = new GameLogicProcessor(es);

			es->RegisterComponentHolderProcessor<GameLogicComponent>(glp, PRIORITY_ESP_GAMELOGIC, UpdateEvent::ALL);
			es->RegisterComponentHolderProcessor<Transform2DComponent>(t2p, PRIORITY_ESP_TRANSFORM2D, UpdateEvent::EndFrame);
			es->RegisterComponentHolderProcessor<Camera2DComponent>(c2p, PRIORITY_ESP_CAMERA2D, UpdateEvent::EndFrame);
			es->RegisterComponentHolderProcessor<Sprite2DComponent>(spr2d, PRIORITY_ESP_SPR2DRENDER, UpdateEvent::EndFrame);
		}

        void AddSystem(System *sys);
		System* getSystem(const std::string& name) const;

    private:
        bool InitSystems();
        void ShutdownSystems();

        std::vector<System*> systems;
		std::vector<System*> systemsToShutdown;
        bool running;
};

}
