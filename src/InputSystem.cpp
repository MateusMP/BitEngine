
#include "InputSystem.h"

#include "MessageChannel.h"

namespace BitEngine{


InputSystem::InputSystem()
    : m_name("Input")
{
    Channel::AddListener<WindowCreated>(this);
}

const std::string& InputSystem::getName() const{
    return m_name;
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
