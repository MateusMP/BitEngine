#include "Core/CommandSystem.h"
#include "Core/Logger.h"

namespace BitEngine
{
	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, int _other)
		: commandID(_id), intensity(_intensity)
	{
		other.other = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, InputReceiver::KeyAction _other)
		: commandID(_id), intensity(_intensity)
	{
		other.fromButton = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, InputReceiver::MouseAction _other, double x, double y)
		: commandID(_id), intensity(_intensity), mouse_x(x), mouse_y(y)
	{
		other.fromMouse = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity)
		: commandID(_id), intensity(_intensity)
	{
		other.other = 0;
	}


	CommandSystem::CommandSystem()
		: System("Command")
	{
		m_commandState = 0;
	}

	CommandSystem::~CommandSystem()
	{

	}

	bool CommandSystem::Init()
	{
		getMessenger()->RegisterListener<InputReceiver::MsgKeyboardInput>(this, BE_MESSAGE_HANDLER(CommandSystem::Message_KeyboardInput));
		getMessenger()->RegisterListener<InputReceiver::MsgMouseInput>(this, BE_MESSAGE_HANDLER(CommandSystem::Message_MouseInput));
		return true;
	}

	void CommandSystem::Shutdown()
	{
		m_commands.clear();
		m_commandState = 0;
	}

	void CommandSystem::Update()
	{

	}

	void CommandSystem::setCommandState(int state)
	{
		m_commandState = state;
	}

	void CommandSystem::UnregisterCommand(int commandID)
	{
		for (auto it = m_commands.begin(); it != m_commands.end(); ++it)
		{
			if (it->second == commandID)
				it = m_commands.erase(it);
		}
	}

	bool CommandSystem::RegisterKeyCommandForAllMods(int commandID, int commandState, int key)
	{
		const int cmd = commandID;
		const int cmdS = commandState;
		uint32 s = 1;
		uint32 f = 0;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::PRESS, InputReceiver::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

		if ( f != 0x7FFF ) {
			LOG(EngineLog, BE_LOG_WARNING) << "Failed to register key for all press mods: " << f << ". Ambiguos key?";
		}

		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, InputReceiver::KeyAction::RELEASE, InputReceiver::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

		if ( (f >> 15) != 0x7FFF ) {
			LOG(EngineLog, BE_LOG_WARNING) << "Failed to register key for all release mods: " << f << ". Ambiguos key?";
		}

		return f == 0x3FFFFFFF;
	}

	bool CommandSystem::RegisterKeyboardCommand(int commandID, int commandState, int key)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.MsgCommandInputType = InputType::keyboard;

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
		idtf.MsgCommandInputType = InputType::keyboard;

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
		idtf.MsgCommandInputType = InputType::mouse;

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

	void CommandSystem::Message_KeyboardInput(const BaseMessage& msg_)
	{
		const InputReceiver::MsgKeyboardInput& msg = static_cast<const InputReceiver::MsgKeyboardInput&>(msg_);

		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);
			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->SendMessage(MsgCommandInput(cmdID, 1, msg.keyAction));
			}
			else
			{
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for input key: " << msg.key << " action: " << (int)(msg.keyAction) << " mod: " << (int)msg.keyMod;
			}
		}

		// Verify for user defined states using current state
		{
			CommandIdentifier idtf(m_commandState, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->SendMessage(MsgCommandInput(cmdID, 1, msg.keyAction));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for input key: " << msg.key << " action: " << (int)(msg.keyAction) << " mod: " << (int)msg.keyMod;
			}
		}
	}

	void CommandSystem::Message_MouseInput(const BaseMessage& msg_)
	{
		const InputReceiver::MsgMouseInput& msg = static_cast<const InputReceiver::MsgMouseInput&>(msg_);
		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->SendMessage(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for this mouse input.";
			}
		}

		// Verify for user defined states using current state
		{
			CommandIdentifier idtf(m_commandState, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->SendMessage(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for this mouse input.";
			}
		}
	}
}
