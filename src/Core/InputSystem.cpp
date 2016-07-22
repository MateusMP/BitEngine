
#include "Core/InputSystem.h"

#include "Core/Logger.h"

namespace BitEngine{

InputSystem::InputSystem(GameEngine* ge, IInputDriver *input)
    : System(ge), driver(input)
{
}

InputSystem::~InputSystem()
{
}

bool InputSystem::Init()
{
	driver->setMessenger(getEngine()->getMessenger());
	driver->Init();

	getEngine()->getMessenger()->RegisterListener<MsgWindowCreated>(this, BE_MESSAGE_HANDLER(InputSystem::Message_WindowCreated));
	getEngine()->getMessenger()->RegisterListener<MsgWindowClosed>(this, BE_MESSAGE_HANDLER(InputSystem::Message_WindowClosed));

	return true;
}

void InputSystem::Update()
{
	driver->poolEvents();
}

void InputSystem::Shutdown()
{
	delete driver;
	driver = nullptr;
}


void InputSystem::Message_WindowCreated(const BaseMessage& wndcr)
{
	const MsgWindowCreated& msg = static_cast<const MsgWindowCreated&>(wndcr);
	driver->inputWindowCreated(msg.window);
}

void InputSystem::Message_WindowClosed(const BaseMessage& wndcr)
{
	const MsgWindowClosed& msg = static_cast<const MsgWindowClosed&>(wndcr);
	driver->inputWindowDestroyed(msg.window);
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

	getMessenger()->SendMessage(MsgKeyboardInput(key, action, (KeyMod)mods));
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

	getMessenger()->SendMessage(MsgMouseInput(button, action, (KeyMod)mods, cursorInScreenX, cursorInScreenY));
}

void InputReceiver::mouseInput(double x, double y)
{
	cursorInScreenX = x;
	cursorInScreenY = y;

	getMessenger()->SendMessage(MsgMouseInput(cursorInScreenX, cursorInScreenY));
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
