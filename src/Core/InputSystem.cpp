
#include "Core/InputSystem.h"

#include "Core/MessageChannel.h"
#include "Core/Logger.h"

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


void InputReceiver::keyboardInput(int key, int scancode, KeyAction action, int mods)
{
	switch (action)
	{
		case KeyAction::REPEAT:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::KFALSE;
		break;

		case KeyAction::PRESS:
			m_keyDown[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
			m_keyReleased[key] = KeyMod::KFALSE;
		break;

		case KeyAction::RELEASE:
			m_keyReleased[key] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
			m_keyDown[key] = KeyMod::KFALSE;
		break;

		default:
			LOG(EngineLog, BE_LOG_WARNING) << "Invalid key action: " << ((int)action);
			return;
	}

	Channel::Broadcast<KeyboardInput>(KeyboardInput(key, action, (KeyMod)mods));
}

void InputReceiver::mouseInput(int button, MouseAction action, int mods)
{
	switch (action)
	{
		case MouseAction::PRESS:
			m_mouseDown[button] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
			m_mouseReleased[button] = KeyMod::KFALSE;
			break;

		case MouseAction::RELEASE:
			m_mouseReleased[button] = (KeyMod)(((unsigned char)KeyMod::KTRUE) | (unsigned char)mods);
			m_mouseDown[button] = KeyMod::KFALSE;
			break;

		default:
			LOG(EngineLog, BE_LOG_WARNING) << "Invalid mouse action: " << ((int)action);
			return;
	}

	Channel::Broadcast<MouseInput>(MouseInput(button, action, (KeyMod)mods, cursorInScreenX, cursorInScreenY));
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

	return KeyMod::KFALSE;
}

InputReceiver::KeyMod InputReceiver::keyReleased(int key)
{
	auto k = m_keyReleased.find(key);
	if (k != m_keyReleased.end())
		return k->second;

	return KeyMod::KFALSE;
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
