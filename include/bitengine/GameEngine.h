#pragma once

#include <vector>

#include "System.h"
#include "MessageType.h"

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
