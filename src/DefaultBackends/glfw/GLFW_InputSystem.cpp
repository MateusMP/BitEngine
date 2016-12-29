#include <unordered_map>

#include "bitengine/Defaultbackends/glfw/GLFW_InputSystem.h"
#include "bitengine/Defaultbackends/glfw/GLFW_VideoSystem.h"

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods);
void GlfwMousePosCallback(GLFWwindow* window, double x, double y);

std::unordered_map<GLFWwindow*, BitEngine::Input::InputReceiver> inputReceivers;

GLFW_InputSystem::GLFW_InputSystem(BitEngine::GameEngine* ge)
	: InputSystem(ge)
{

}

bool GLFW_InputSystem::Init()
{
	getEngine()->getMessenger()->registerListener<BitEngine::MsgWindowCreated>(this);
	getEngine()->getMessenger()->registerListener<BitEngine::MsgWindowClosed>(this);
	return true;
}

void GLFW_InputSystem::Shutdown()
{
}

void GLFW_InputSystem::Update()
{
	glfwPollEvents();
}

BitEngine::Input::KeyMod GLFW_InputSystem::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::Input::KeyMod::KFALSE;

	return w->second.isKeyPressed(key);
}

BitEngine::Input::KeyMod GLFW_InputSystem::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return BitEngine::Input::KeyMod::KFALSE;

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

void GLFW_InputSystem::onMessage(const BitEngine::MsgWindowCreated& msg)
{
	GLFW_VideoSystem::Window_glfw* w = static_cast<GLFW_VideoSystem::Window_glfw*>(msg.window);

	// Creates instance for this window
	inputReceivers.emplace(w->m_glfwWindow, getEngine()->getMessenger());

	// Define callback for functions
	glfwSetKeyCallback(w->m_glfwWindow, GlfwKeyCallback);
	glfwSetMouseButtonCallback(w->m_glfwWindow, GlfwMouseCallback);
	glfwSetCursorPosCallback(w->m_glfwWindow, GlfwMousePosCallback);
}

void GLFW_InputSystem::onMessage(const BitEngine::MsgWindowClosed& msg)
{
	inputReceivers.erase(static_cast<GLFW_VideoSystem::Window_glfw*>(msg.window)->m_glfwWindow);
}


// ******************
// GLFW CALLBACKS
// ******************

void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN) {
		LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	}

	// LOGCLASS(VERBOSE) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods;
	
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		BitEngine::Input::KeyAction act = BitEngine::Input::KeyAction::NONE;
		switch (action) 
		{
			case GLFW_REPEAT:
				act = BitEngine::Input::KeyAction::REPEAT;
				break;
			case GLFW_PRESS:
				act = BitEngine::Input::KeyAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = BitEngine::Input::KeyAction::RELEASE;
				break;
			default:
				LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Invalid key action: " << action;
				return;
		}
		it->second.keyboardInput(key, scancode, act, mods);
	}
	else {
		LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Invalid window input!";
	}
}

void GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) 
	{
		BitEngine::Input::MouseAction act = BitEngine::Input::MouseAction::NONE;
		switch (action)
		{
			case GLFW_PRESS:
				act = BitEngine::Input::MouseAction::PRESS;
				break;
			case GLFW_RELEASE:
				act = BitEngine::Input::MouseAction::RELEASE;
				break;
			default:
				LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Invalid mouse action: " << action;
				return;
		}
		it->second.mouseInput(button, act, mods);
	}
	else 
	{
		LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Invalid window input!";
	}
}

void GlfwMousePosCallback(GLFWwindow* window, double x, double y) 
{
	auto it = inputReceivers.find(window);
	if (it != inputReceivers.end()) {
		it->second.mouseInput(x, y);
	} else {
		LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Invalid window input!";
	}
}