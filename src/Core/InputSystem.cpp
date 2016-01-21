
#include "Core/InputSystem.h"

#include "Core/MessageChannel.h"
#include "Core/EngineLoggers.h"

#include "DefaultBackends/glfw/GLFW_VideoDriver.h"

namespace BitEngine{

InputSystem::InputSystem(IInputDriver *input)
    : System("Input"), driver(input)
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
	driver->poolEvents();
}

void InputSystem::Shutdown()
{
	delete driver;
}


void InputSystem::Message(const WindowCreated& wndcr)
{
	driver->inputWindowCreated(wndcr.window);
}

void InputSystem::Message(const WindowClosed& wndcr)
{
	driver->inputWindowDestroyed(wndcr.window);
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
