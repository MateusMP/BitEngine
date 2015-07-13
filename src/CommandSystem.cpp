
#include "CommandSystem.h"
#include "EngineLoggers.h"

namespace BitEngine{

	CommandSystem::CommandSystem()
		: System("Command")
	{
		m_commandState = 0;
		Channel::AddListener<InputReceiver::KeyboardInput>(this);
	}

	CommandSystem::~CommandSystem()
	{

	}

	bool CommandSystem::Init()
	{
		return true;
	}

	void CommandSystem::Shutdown()
	{
	}

	void CommandSystem::Update()
	{

	}

	void CommandSystem::setCommandState(int state)
	{
		m_commandState = state;
	}

	bool CommandSystem::RegisterKeyboardCommand(int commandID, int commandState, int key)
	{
		CommandIdentifier idtf(commandState, InputType::keyboard);
		idtf.keyboard.key = key;
		idtf.keyboard.keyAction = InputReceiver::KeyAction::PRESS;
		idtf.keyboard.keyMod = InputReceiver::KeyMod::NONE;

		// Search for PRESS
		auto it = m_commands.find(idtf);
		if (it == m_commands.end())
		{
			// Search for RELEASE
			idtf.keyboard.keyAction = InputReceiver::KeyAction::RELEASE;
			it = m_commands.find(idtf);
			if (it == m_commands.end())
			{
				// Register for RELEASE
				m_commands[idtf] = commandID;

				// Register for PRESS
				idtf.keyboard.keyAction = InputReceiver::KeyAction::PRESS;
				m_commands[idtf] = commandID;
				return true;
			}
		}

		return false;
	}

	bool CommandSystem::RegisterKeyboardCommand(int commandID, int commandState, int key, InputReceiver::KeyAction action, InputReceiver::KeyMod mod)
	{
		CommandIdentifier idtf(commandState, InputType::keyboard);
		idtf.keyboard.key = key;
		idtf.keyboard.keyAction = action;
		idtf.keyboard.keyMod = mod;

		auto it = m_commands.find(idtf);
		if (it == m_commands.end()){
			m_commands[idtf] = commandID;
			return true;
		}

		return false;
	}

	void CommandSystem::Message(const InputReceiver::KeyboardInput& msg)
	{
		CommandIdentifier idtf(m_commandState, InputType::keyboard);
		idtf.keyboard.key = msg.key;
		idtf.keyboard.keyAction = msg.keyAction;
		idtf.keyboard.keyMod = msg.keyMod;

		auto it = m_commands.find(idtf);
		if (it != m_commands.end()){
			const int cmdID = m_commands[idtf];

			LOGTO(Verbose) << "Command dispatch: " << cmdID << endlog;

			Channel::Broadcast<CommandInput>(CommandInput(cmdID, 1, msg.keyAction));
		}
		else {
			// LOGTO(Verbose) << "No command for this input." << endlog;
		}
	}

}