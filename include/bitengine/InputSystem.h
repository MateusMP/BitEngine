#pragma once

#include "Graphics.h"

#include "System.h"
#include "MessageChannel.h"
#include "MessageType.h"

namespace BitEngine{

class InputSystem
    : public System
{
    public:
        InputSystem();
        ~InputSystem();
		
        bool Init() override;
        void Shutdown() override;
        void Update() override;

        void Message(const WindowCreated& wndcr);

    private:
        static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

};


}

