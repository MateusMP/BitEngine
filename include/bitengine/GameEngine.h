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

    private:
        bool InitSystems();
        void ShutdownSystems();

        std::vector<System*> systems;
        bool running;
};

}
