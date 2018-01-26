#include "bitengine/Core/CommandSystem.h"
#include "bitengine/Core/Logger.h"

namespace BitEngine
{
	std::size_t CommandSystem::CIHash::operator()(const CommandIdentifier& k) const
	{
		if (k.msgCommandInputType == InputType::keyboard)
			return (std::hash<int>()(k.commandState << 24)
					^ (std::hash<int>()((int) k.msgCommandInputType) << 16))
					^ (std::hash<int>()(k.keyboard.key) << 8)
					^ (std::hash<int>()((int) k.keyboard.keyAction) << 4)
					^ (std::hash<int>()((int) k.keyboard.keyMod));
		else if (k.msgCommandInputType == InputType::mouse)
			return (std::hash<int>()(k.commandState << 24)
					^ (std::hash<int>()((int) k.msgCommandInputType) << 16))
					^ (std::hash<int>()(k.mouse.button) << 8)
					^ (std::hash<int>()((int) k.mouse.action) << 4)
					^ (std::hash<int>()((int) k.mouse.keyMod));

		return 0;
	}
	bool CommandSystem::CIEqual::operator()(const CommandIdentifier& t1, const CommandIdentifier& t2) const
	{
		if (t1.msgCommandInputType == InputType::keyboard && t2.msgCommandInputType == InputType::keyboard)
			return (t1.commandState == t2.commandState
					&& t1.keyboard.key == t2.keyboard.key
					&& t1.keyboard.keyAction == t2.keyboard.keyAction
					&& t1.keyboard.keyMod == t2.keyboard.keyMod);
		else if (t1.msgCommandInputType == InputType::mouse && t2.msgCommandInputType == InputType::mouse)
			return (t1.commandState == t2.commandState
					&& t1.mouse.button == t2.mouse.button
					&& t1.mouse.action == t2.mouse.action
					&& t1.mouse.keyMod == t2.mouse.keyMod);

		return false;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, int _other)
		: commandID(_id), intensity(_intensity), mouse_x(0), mouse_y(0)
	{
		action.other = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, Input::KeyAction _other)
		: commandID(_id), intensity(_intensity), mouse_x(0), mouse_y(0)
	{
		action.fromButton = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, Input::MouseAction _other, double x, double y)
		: commandID(_id), intensity(_intensity), mouse_x(x), mouse_y(y)
	{
		action.fromMouse = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity)
		: commandID(_id), intensity(_intensity), mouse_x(0), mouse_y(0)
	{
		action.other = 0;
	}


	CommandSystem::CommandSystem(Messenger* m)
		: MessengerEndpoint(m)
	{
		m_commandState = 0;

		subscribe<Input::MsgKeyboardInput>(&CommandSystem::onMessage, this);
		subscribe<Input::MsgMouseInput>(&CommandSystem::onMessage, this);
	}

	CommandSystem::~CommandSystem()
	{
	}

	bool CommandSystem::init()
	{
		return true;
	}

	void CommandSystem::shutdown()
	{
		m_commands.clear();
		m_commandState = 0;
	}

	void CommandSystem::setCommandState(int state)
	{
		m_commandState = state;
	}

	void CommandSystem::unregisterCommand(int commandID)
	{
		for (auto it = m_commands.begin(); it != m_commands.end(); ++it)
		{
			if (it->second == commandID) {
				it = m_commands.erase(it);
			}
		}
	}

	bool CommandSystem::registerKeyCommandForAllMods(int commandID, int commandState, int key)
	{
		const int cmd = commandID;
		const int cmdS = commandState;
		u32 s = 1;
		u32 f = 0;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

		if ( f != 0x7FFF ) {
			LOG(EngineLog, BE_LOG_WARNING) << "Failed to register key for all press mods: " << f << ". Ambiguos key?";
		}

		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (registerKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

		if ( (f >> 15) != 0x7FFF ) {
			LOG(EngineLog, BE_LOG_WARNING) << "Failed to register key for all release mods: " << f << ". Ambiguos key?";
		}

		return f == 0x3FFFFFFF;
	}

	bool CommandSystem::registerKeyboardCommand(int commandID, int commandState, int key)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.msgCommandInputType = InputType::keyboard;

		idtf.keyboard.key = key;
		idtf.keyboard.keyAction = Input::KeyAction::PRESS;
		idtf.keyboard.keyMod = Input::KeyMod::NONE;

		// Search for PRESS
		if (m_commands.find(idtf) == m_commands.end())
		{
			// Search for RELEASE
			idtf.keyboard.keyAction = Input::KeyAction::RELEASE;
			if (m_commands.find(idtf) == m_commands.end())
			{
				// Register for RELEASE
				m_commands[idtf] = commandID;

				// Register for PRESS
				idtf.keyboard.keyAction = Input::KeyAction::PRESS;
				m_commands[idtf] = commandID;
				return true;
			}
		}

		return false;
	}

	bool CommandSystem::registerKeyboardCommand(int commandID, int commandState, int key, Input::KeyAction action, Input::KeyMod mod /*= KeyMod::NONE*/)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.msgCommandInputType = InputType::keyboard;

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

	bool CommandSystem::RegisterMouseCommand(int commandID, int commandState, int button, Input::MouseAction action, Input::KeyMod mod /*= Input::KeyMod::NONE*/)
	{
		CommandIdentifier idtf;
		idtf.commandState = commandState;
		idtf.msgCommandInputType = InputType::mouse;

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

	void CommandSystem::onMessage(const Input::MsgKeyboardInput& msg)
	{
		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);
			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->emit(MsgCommandInput(cmdID, 1, msg.keyAction));
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

				getMessenger()->emit(MsgCommandInput(cmdID, 1, msg.keyAction));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for input key: " << msg.key << " action: " << (int)(msg.keyAction) << " mod: " << (int)msg.keyMod;
			}
		}
	}

	void CommandSystem::onMessage(const Input::MsgMouseInput& msg)
	{
		// Verify for global commands
		{
			CommandIdentifier idtf(-1, msg);

			auto it = m_commands.find(idtf);
			if (it != m_commands.end())
			{
				const int cmdID = m_commands[idtf];

				LOG(EngineLog, BE_LOG_VERBOSE) << "Command dispatch: " << cmdID;

				getMessenger()->emit(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
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

				getMessenger()->emit(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for this mouse input.";
			}
		}
	}
}
