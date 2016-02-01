#include "Defaultbackends/glfw/GLFW_InputDriver.h"

std::unordered_map<GLFWwindow*, BitEngine::InputReceiver> GLFW_InputDriver::inputReceivers;

void GLFW_InputDriver::inputWindowCreated(BitEngine::Window* window)
{
	GLFW_VideoDriver::Window_glfw* w = static_cast<GLFW_VideoDriver::Window_glfw*>(window);

	// Creates instance for this window
	inputReceivers[w->m_glfwWindow];

	// Define callback for functions
	glfwSetKeyCallback(w->m_glfwWindow, GlfwKeyCallback);
	glfwSetMouseButtonCallback(w->m_glfwWindow, GlfwMouseCallback);
	glfwSetCursorPosCallback(w->m_glfwWindow, GlfwMousePosCallback);
}

void GLFW_InputDriver::inputWindowDestroyed(BitEngine::Window* window)
{
	inputReceivers.erase(static_cast<GLFW_VideoDriver::Window_glfw*>(window)->m_glfwWindow);
}


BitEngine::InputReceiver::KeyMod GLFW_InputDriver::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::InputReceiver::KeyMod::KFALSE;

	return w->second.isKeyPressed(key);
}

BitEngine::InputReceiver::KeyMod GLFW_InputDriver::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::InputReceiver::KeyMod::KFALSE;

	return w->second.keyReleased(key);
}

double GLFW_InputDriver::getMouseX() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseX();
}

double GLFW_InputDriver::getMouseY() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseY();
}

void GLFW_InputDriver::poolEvents()
{
	glfwPollEvents();
}

void GLFW_InputDriver::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN) {
		LOGCLASS(BE_LOG_WARNING) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	}

	// LOGCLASS(VERBOSE) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		BitEngine::InputReceiver::KeyAction act = BitEngine::InputReceiver::KeyAction::NONE;
		switch (action) 
		{
			case GLFW_REPEAT:
				act = BitEngine::InputReceiver::KeyAction::REPEAT;
				break;
			case GLFW_PRESS:
				act = BitEngine::InputReceiver::KeyAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = BitEngine::InputReceiver::KeyAction::RELEASE;
				break;
			default:
				LOGCLASS(BE_LOG_WARNING) << "Invalid key action: " << action;
				return;
		}
		it->second.keyboardInput(key, scancode, act, mods);
	}
	else {
		LOGCLASS(BE_LOG_ERROR) << "Invalid window input!";
	}
}

void GLFW_InputDriver::GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		BitEngine::InputReceiver::MouseAction act = BitEngine::InputReceiver::MouseAction::NONE;
		switch (action)
		{
			case GLFW_PRESS:
				act = BitEngine::InputReceiver::MouseAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = BitEngine::InputReceiver::MouseAction::RELEASE;
				break;
			default:
				LOGCLASS(BE_LOG_WARNING) << "Invalid mouse action: " << action;
				return;
		}
		it->second.mouseInput(button, act, mods);
	}
	else 
	{
		LOGCLASS(BE_LOG_ERROR) << "Invalid window input!";
	}
}

void GLFW_InputDriver::GlfwMousePosCallback(GLFWwindow* window, double x, double y) 
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) {
		it->second.mouseInput(x, y);
	}
	else {
		LOGCLASS(BE_LOG_ERROR) << "Invalid window input!";
	}
}
