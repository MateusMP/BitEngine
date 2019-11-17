#include <unordered_map>

#include "Platform/glfw/GLFW_InputSystem.h"
#include "Platform/glfw/GLFW_VideoSystem.h"

namespace BitEngine {

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods);
void GlfwMousePosCallback(GLFWwindow* window, double x, double y);

std::unordered_map<GLFWwindow*, Input::InputReceiver> inputReceivers;

GLFW_InputSystem::GLFW_InputSystem()
{

}

bool GLFW_InputSystem::init()
{
	return true;
}

void GLFW_InputSystem::shutdown()
{
}

void GLFW_InputSystem::update()
{
	glfwPollEvents();
}

KeyMod GLFW_InputSystem::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return KeyMod::KFALSE;

	return w->second.isKeyPressed(key);
}

KeyMod GLFW_InputSystem::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return KeyMod::KFALSE;

	return w->second.keyReleased(key);
}

double GLFW_InputSystem::getMouseX() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseX();
}

double GLFW_InputSystem::getMouseY() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseY();
}

void GLFW_InputSystem::registerWindow(Window* window)
{
    GLFW_Window* be_window = ((GLFW_Window*)window);
	GLFWwindow *glfwWindow = be_window->window;
	// Creates instance for this window
	inputReceivers.emplace(glfwWindow, window);

	// Define callback for functions
	glfwSetKeyCallback(glfwWindow, GlfwKeyCallback);
	glfwSetMouseButtonCallback(glfwWindow, GlfwMouseCallback);
	glfwSetCursorPosCallback(glfwWindow, GlfwMousePosCallback);
}

void GLFW_InputSystem::unregisterWindow(Window* window)
{
	GLFWwindow *glfwWindow = ((GLFW_Window*)window)->window;
	inputReceivers.erase(glfwWindow);
}


// ******************
// GLFW CALLBACKS
// ******************

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN) {
		LOG(EngineLog, BE_LOG_WARNING) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	}

	// LOGCLASS(VERBOSE) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		KeyAction act = KeyAction::NONE;
		switch (action) 
		{
			case GLFW_REPEAT:
				act = KeyAction::REPEAT;
				break;
			case GLFW_PRESS:
				act = KeyAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = KeyAction::RELEASE;
				break;
			default:
				LOG(EngineLog, BE_LOG_WARNING) << "Invalid key action: " << action;
				return;
		}
		it->second.keyboardInput(key, scancode, act, mods);
	}
	else {
		LOG(EngineLog, BE_LOG_WARNING) << "Invalid window input!";
	}
}

void GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		MouseAction act = MouseAction::NONE;
		switch (action)
		{
			case GLFW_PRESS:
				act = MouseAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = MouseAction::RELEASE;
				break;
			default:
				LOG(EngineLog, BE_LOG_WARNING) << "Invalid mouse action: " << action;
				return;
		}
		it->second.mouseInput(button, act, mods);
	}
	else 
	{
		LOG(EngineLog, BE_LOG_ERROR) << "Invalid window input!";
	}
}

void GlfwMousePosCallback(GLFWwindow* window, double x, double y) 
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) {
		it->second.mouseInput(x, y);
	} else {
		LOG(EngineLog, BE_LOG_ERROR) << "Invalid window input!";
	}
}

}