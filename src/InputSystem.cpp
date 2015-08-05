
#include "InputSystem.h"

#include "MessageChannel.h"
#include "EngineLoggers.h"

namespace BitEngine{

std::unordered_map<Window*, InputReceiver> InputSystem::inputReceivers;


InputSystem::InputSystem()
    : System("Input")
{
    Channel::AddListener<WindowCreated>(this);
	Channel::AddListener<WindowClosed>(this);
}

InputSystem::~InputSystem(){
}

bool InputSystem::Init()
{
	return true;
}

void InputSystem::Update()
{
	PoolEvents();
}

void InputSystem::Shutdown()
{

}

Window* InputSystem::FindGLFWwindow(GLFWwindow* window)
{
	for (auto& it = inputReceivers.begin(); it != inputReceivers.end(); ++it){
		Window_glfw* glfwW = static_cast<Window_glfw*>(it->first);
		if (glfwW->m_glfwWindow == window){
			return glfwW;
		}
	}

	return nullptr;
}

void InputSystem::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN){
		LOGTO(Warning) << "Unknown key: scancode: " << scancode << " Action: " << action << " Mods: " << mods << endlog;
	}

	// LOGTO(Verbose) << "Key Input on window " << window << " key: " << key << " scancode: " << scancode << " Action: " << action << " Mods: " << mods << endlog;
	Window *w = FindGLFWwindow(window);

	if (w == nullptr){
		LOGTO(Warning) << "Unhandled key callback!" << endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()){
		it->second.keyboardInput(key, scancode, action, mods);
	} else {
		LOGTO(Error) << "Invalid window input!" << endlog;
	}
}

void InputSystem::GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	Window* w = FindGLFWwindow(window);

	if (w == nullptr){
		LOGTO(Warning) << "Unhandled mouse callback!" << endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()){
		it->second.mouseInput(button, action, mods);
	} else {
		LOGTO(Error) << "Invalid window input!" << endlog;
	}
}

void InputSystem::GlfwMousePosCallback(GLFWwindow* window, double x, double y){
	Window* w = FindGLFWwindow(window);

	if (w == nullptr){
		LOGTO(Warning) << "Unhandled mouse pos callback!" << endlog;
		return;
	}

	auto it = inputReceivers.find(w);
	if (it != inputReceivers.end()){
		it->second.mouseInput(x, y);
	}
	else {
		LOGTO(Error) << "Invalid window input!" << endlog;
	}
}

void InputSystem::Message(const WindowCreated& wndcr)
{
	// Creates instance for this window
	inputReceivers[wndcr.window];

	const Window_glfw* glfwW = static_cast<const Window_glfw*>(wndcr.window);

	// Define callback for functions
	glfwSetKeyCallback(glfwW->m_glfwWindow, GlfwKeyCallback);
	glfwSetMouseButtonCallback(glfwW->m_glfwWindow, GlfwMouseCallback);
	glfwSetCursorPosCallback(glfwW->m_glfwWindow, GlfwMousePosCallback);
}

void InputSystem::Message(const WindowClosed& wndcr)
{
	const Window_glfw* glfwW = static_cast<const Window_glfw*>(wndcr.window);

	Window *w = FindGLFWwindow(glfwW->m_glfwWindow);

	inputReceivers.erase(w);
}

InputReceiver::KeyMod InputSystem::isKeyPressed(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return InputReceiver::KeyMod::FALSE;

	return w->second.isKeyPressed(key);
}

InputReceiver::KeyMod InputSystem::keyReleased(int key)
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return InputReceiver::KeyMod::FALSE;

	return w->second.keyReleased(key);
}

double InputSystem::getMouseX() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseX();
}

double InputSystem::getMouseY() const
{
	auto w = inputReceivers.begin();
	if (w == inputReceivers.end())
		return -1;

	return w->second.getMouseY();
}

void InputSystem::PoolEvents()
{
	glfwPollEvents();
}

void InputReceiver::keyboardInput(int key, int scancode, int action, int mods)
{
	KeyAction act = KeyAction::NONE;
	switch (action){
		case GLFW_REPEAT:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::FALSE;
			act = KeyAction::REPEAT;
		break;

		case GLFW_PRESS:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::FALSE;
			act = KeyAction::PRESS;
		break;

		case GLFW_RELEASE:
			m_keyReleased[key] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_keyDown[key] = KeyMod::FALSE;
			act = KeyAction::RELEASE;
		break;

		default:
			LOGTO(Warning) << "Invalid key action: " << action << endlog;
			return;
	}

	Channel::Broadcast<KeyboardInput>(KeyboardInput(key, act, (KeyMod)mods));
}

void InputReceiver::mouseInput(int button, int action, int mods)
{
	MouseAction act = MouseAction::NONE;
	switch (action){
		case GLFW_PRESS:
			m_mouseDown[button] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_mouseReleased[button] = KeyMod::FALSE;
			act = MouseAction::PRESS;
			break;

		case GLFW_RELEASE:
			m_mouseReleased[button] = (KeyMod)(((unsigned char)KeyMod::TRUE) | (unsigned char)mods);
			m_mouseDown[button] = KeyMod::FALSE;
			act = MouseAction::RELEASE;
			break;

		default:
			LOGTO(Warning) << "Invalid mouse action: " << action << endlog;
			return;
	}

	Channel::Broadcast<MouseInput>(MouseInput(button, act, (KeyMod)mods, cursorInScreenX, cursorInScreenY));
}

void InputReceiver::mouseInput(double x, double y)
{
	cursorInScreenX = x;
	cursorInScreenY = y;

	Channel::Broadcast<MouseInput>(MouseInput(cursorInScreenX, cursorInScreenY));
}

InputReceiver::KeyMod InputReceiver::isKeyPressed(int key)
{
	auto k = m_keyDown.find(key);
	if (k != m_keyDown.end())
		return k->second;

	return KeyMod::FALSE;
}

InputReceiver::KeyMod InputReceiver::keyReleased(int key)
{
	auto k = m_keyReleased.find(key);
	if (k != m_keyReleased.end())
		return k->second;

	return KeyMod::FALSE;
}

double InputReceiver::getMouseX() const
{
	return cursorInScreenX;
}

double InputReceiver::getMouseY() const
{
	return cursorInScreenY;
}

}
