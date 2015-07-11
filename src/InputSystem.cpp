
#include "InputSystem.h"

#include "MessageChannel.h"

namespace BitEngine{


InputSystem::InputSystem()
    : System("Input")
{
    Channel::AddListener<WindowCreated>(this);
}

InputSystem::~InputSystem(){
}

void InputSystem::Shutdown()
{

}

void InputSystem::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_SPACE:
            glfwSetTime(0.0);
            break;
    }
}

void InputSystem::Message(const WindowCreated& wndcr)
{
    glfwSetKeyCallback(wndcr.window, GlfwKeyCallback);
    printf("MENSAGEM RECEBIDA: WindowCreated\n");
}

bool InputSystem::Init()
{
    return true;
}

void InputSystem::Update()
{

}


}
