#include "Core/CommandSystem.h"
#include "Core/Logger.h"

namespace BitEngine
{
	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, int _other)
		: commandID(_id), intensity(_intensity)
	{
		other.other = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, Input::KeyAction _other)
		: commandID(_id), intensity(_intensity)
	{
		other.fromButton = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity, Input::MouseAction _other, double x, double y)
		: commandID(_id), intensity(_intensity), mouse_x(x), mouse_y(y)
	{
		other.fromMouse = _other;
	}

	CommandSystem::MsgCommandInput::MsgCommandInput(int _id, float _intensity)
		: commandID(_id), intensity(_intensity)
	{
		other.other = 0;
	}


	CommandSystem::CommandSystem(GameEngine* ge)
		: System(ge)
	{
		m_commandState = 0;

		getEngine()->getMessenger()->registerListener<Input::MsgKeyboardInput>(this);
		getEngine()->getMessenger()->registerListener<Input::MsgMouseInput>(this);
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
		u32 s = 1;
		u32 f = 0;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::PRESS, Input::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

		if ( f != 0x7FFF ) {
			LOG(EngineLog, BE_LOG_WARNING) << "Failed to register key for all press mods: " << f << ". Ambiguos key?";
		}

		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::NONE)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_ALT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_CTRL)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SUPER_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::ALT_SHIFT_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::SHIFT_CTRL_SUPER)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT_SHIFT)) { f |= s; } s <<= 1;
		if (RegisterKeyboardCommand(cmd, cmdS, key, Input::KeyAction::RELEASE, Input::KeyMod::CTRL_ALT_SUPER)) { f |= s; } s <<= 1;

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
		idtf.keyboard.keyAction = Input::KeyAction::PRESS;
		idtf.keyboard.keyMod = Input::KeyMod::NONE;

		// Search for PRESS
		auto it = m_commands.find(idtf);
		if (it == m_commands.end())
		{
			// Search for RELEASE
			idtf.keyboard.keyAction = Input::KeyAction::RELEASE;
			it = m_commands.find(idtf);
			if (it == m_commands.end())
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

	bool CommandSystem::RegisterKeyboardCommand(int commandID, int commandState, int key, Input::KeyAction action, Input::KeyMod mod /*= KeyMod::NONE*/)
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

	bool CommandSystem::RegisterMouseCommand(int commandID, int commandState, int button, Input::MouseAction action, Input::KeyMod mod /*= Input::KeyMod::NONE*/)
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

				getEngine()->getMessenger()->dispatch(MsgCommandInput(cmdID, 1, msg.keyAction));
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

				getEngine()->getMessenger()->dispatch(MsgCommandInput(cmdID, 1, msg.keyAction));
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

				getEngine()->getMessenger()->dispatch(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
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

				getEngine()->getMessenger()->dispatch(MsgCommandInput(cmdID, 1, msg.action, msg.x, msg.y));
			}
			else {
				// LOG(EngineLog, BE_LOG_VERBOSE) << "No command for this mouse input.";
			}
		}
	}
}
