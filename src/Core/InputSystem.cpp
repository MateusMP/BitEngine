
#include "Core/InputSystem.h"

#include "Core/Messenger.h"

#include "Core/Logger.h"

namespace BitEngine{

	InputSystem::InputSystem(GameEngine* ge, Input::IInputDriver *input)
		: System(ge), driver(input)
	{
	}

	InputSystem::~InputSystem()
	{
	}

	bool InputSystem::Init()
	{
		driver->Init();

		getEngine()->getMessenger()->registerListener<MsgWindowCreated>(this);
		getEngine()->getMessenger()->registerListener<MsgWindowClosed>(this);

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


	void InputSystem::onMessage(const MsgWindowCreated& msg)
	{
		driver->inputWindowCreated(msg.window);
	}

	void InputSystem::onMessage(const MsgWindowClosed& msg)
	{
		driver->inputWindowDestroyed(msg.window);
	}

}