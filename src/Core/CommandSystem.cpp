
#include "Core/CommandSystem.h"
#include "Core/EngineLoggers.h"

namespace BitEngine
{
	CommandSystem::CommandInput::CommandInput(int _id, float _intensity, int _other)
		: commandID(_id), intensity(_intensity)
	{
		other.other = _other;
	}

	CommandSystem::CommandInput::CommandInput(int _id, float _intensity, InputReceiver::KeyAction _other)
		: commandID(_id), intensity(_intensity)
	{
		other.fromButton = _other;
	}

	CommandSystem::CommandInput::CommandInput(int _id, float _intensity, InputReceiver::MouseAction _other, double x, double y)
		: commandID(_id), intensity(_intensity), mouse_x(x), mouse_y(y)
	{
		other.fromMouse = _other;
	}

	CommandSystem::CommandInput::CommandInput(int _id, float _intensity)
		: commandID(_id), intensity(_intensity)
	{
		other.other = 0;
	}


	CommandSystem::CommandSystem()
		: System("Command")
	{
		m_commandState = 0;
		Channel::AddListener<InputReceiver::KeyboardInput>(this);
		Channel::AddListener<InputReceiver::MouseInput>(this);
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
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.commandInputType = InputType::keyboard;

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

	bool CommandSystem::RegisterKeyboardCommand(int commandID, int commandState, int key, InputReceiver::KeyAction action, InputReceiver::KeyMod mod /*= KeyMod::NONE*/)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.commandInputType = InputType::keyboard;

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

	bool CommandSystem::RegisterMouseCommand(int commandID, int commandState, int button, InputReceiver::MouseAction action, InputReceiver::KeyMod mod /*= InputReceiver::KeyMod::NONE*/)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.commandInputType = InputType::mouse;

		idtf.mouse.button = button;
		idtf.mouse.action = action;
		idtf.mouse.keyMod = mod;

		auto it = m_commands.find(idtf);
		if (it == m_commands.end()){
			m_commands[idtf] = commandID;
			return true;
		}

		return false;
	}

	void CommandSystem::Message(const InputReceiver::KeyboardInput& msg)
	{
		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);
			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOGTO(Verbose) << "Command dispatch: " << cmdID << endlog;

				Channel::Broadcast<CommandInput>(CommandInput(cmdID, 1, msg.keyAction));
			}
			else 
			{
				LOGTO(Verbose) << "No command for this input." << endlog;
			}
		}

		// Verify for user defined states using current state
		{
			CommandIdentifier idtf(m_commandState, msg);

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

	void CommandSystem::Message(const InputReceiver::MouseInput& msg)
	{
		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end()){
				const int cmdID = m_commands[idtf];

				LOGTO(Verbose) << "Command dispatch: " << cmdID << endlog;

				Channel::Broadcast<CommandInput>(CommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOGTO(Verbose) << "No command for this input." << endlog;
			}
		}

		// Verify for user defined states using current state
		{
			CommandIdentifier idtf(m_commandState, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end()){
				const int cmdID = m_commands[idtf];

				LOGTO(Verbose) << "Command dispatch: " << cmdID << endlog;

				Channel::Broadcast<CommandInput>(CommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOGTO(Verbose) << "No command for this input." << endlog;
			}
		}
	}
}