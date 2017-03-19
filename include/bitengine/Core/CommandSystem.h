#pragma once

#include <unordered_map>

#include "bitengine/Core/System.h"
#include "bitengine/Core/InputSystem.h"
#include "bitengine/Core/Message.h"

namespace BitEngine{

	/**
	 * Command System is a default system used to define commands for inputs.
	 * The inputs should be registered to a command, that will be triggered by an event.
	 */
	class CommandSystem : public System
	{
		public:
			/**
			 * The body for command events.
			 */
			struct MsgCommandInput
			{
				public:
					MsgCommandInput(int _id, float _intensity, int _other);
					MsgCommandInput(int _id, float _intensity, Input::KeyAction _other);
					MsgCommandInput(int _id, float _intensity, Input::MouseAction _other, double x, double y);
					MsgCommandInput(int _id, float _intensity);

					const int commandID; // The registered command id
					const float intensity; // If it was an analogic input
					double mouse_x; // Invalid unless Mouse input was used
					double mouse_y; // Invalid unless Mouse input was used

					union {
						Input::KeyAction fromButton; // keyboard / mouse / joystick buttons
						Input::MouseAction fromMouse;
						int other;
					} action; // If there was some other action
			};

			CommandSystem(GameEngine* ge);
			~CommandSystem();

			const char* getName() const override {
				return "Command";
			}

			bool Init() override;
			void Shutdown() override;
			void Update() override;

			void setCommandState(int state);

			/**
			 * Remove all keybinds to given command
			 */
			void UnregisterCommand(int commandID);

			/**
			 * @param commandID	the command ID returned when given key/action/mod combination ocurrs
			 * @param commandState		CommandSystem state necessary for processing this command
			 *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			 * @param key	keyboard key needed
			 * Note that this function will register the commandID for both inputs: RELEASE and PRESS
			 */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key);

			/**
			 * @param commandID	the command ID returned when given key/action/mod combination ocurrs
			 * @param commandState		CommandSystem state necessary for processing this command
			 *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			 * @param key	keyboard key needed
			 * Note that this function will register the commandID for both inputs: RELEASE and PRESS and to be accepted with any keymod combination
			 *
			 * If an ambiguous keybind is found, it wont be overwritten and the command being bind will not be fully binded.
			 * In this case, the function returns false.
			 * It's recommended that all commands for the requested commandID are cleared if this function fails.
			 * Left optional for the caller, so no other binding will be silently removed.
			 */
			bool RegisterKeyCommandForAllMods(int commandID, int commandState, int key);

			/**
			 * @param commandID	Command ID returned when given key/action/mod combination ocurrs
			 * @param commandState		CommandSystem state necessary for processing this command
			 *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			 * @param key	Keyboard key needed
			 * @param action	Key state needed
			 * @param mod	Key modifiers (Shift, Alt, Ctrl, Super)
			 */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key, Input::KeyAction action, Input::KeyMod mod = Input::KeyMod::NONE);

			/**
			 * Same for mouse commands
			 */
			bool RegisterMouseCommand(int commandID, int commandState, int button, Input::MouseAction action, Input::KeyMod mod = Input::KeyMod::NONE);

			/**
			 *
			 * @param commandID
			 * @param commandState
			 * @param mod
			 * @return
			 */
			bool RegisterMouseMove(int commandID, int commandState, Input::KeyMod mod = Input::KeyMod::NONE){
				return RegisterMouseCommand(commandID, commandState, 0, Input::MouseAction::MOVE, mod);
			}

			/**
			 * The command system listen to Input::MsgKeyboardInput so it can check for command triggers.
			 */
			void onMessage(const Input::MsgKeyboardInput& msg_);

			/**
			 * The command system listen to Input::MsgKeyboardInput so it can check for command triggers.
			 */
			void onMessage(const Input::MsgMouseInput& msg);

		private:
			enum class InputType : char{
				keyboard,
				mouse,
				joystick,
				other,
			};

			struct CommandIdentifier{
				CommandIdentifier()
					: commandState(0), msgCommandInputType(InputType::other)
				{}

				CommandIdentifier(int s, const Input::MsgKeyboardInput& k)
					: commandState(s), msgCommandInputType(InputType::keyboard), keyboard(k){}

				CommandIdentifier(int s, const Input::MsgMouseInput& m)
					: commandState(s), msgCommandInputType(InputType::mouse), mouse(m){}

				int commandState;
				InputType msgCommandInputType;

				Input::MsgKeyboardInput keyboard;
				Input::MsgMouseInput mouse;
			};

			// CommandIdentifier Hash and Equal
			class CIHash{
			public:
				std::size_t operator()(const CommandIdentifier& k) const;
			};
			class CIEqual{
			public:
				bool operator() (const CommandIdentifier& t1, const CommandIdentifier& t2) const;
			};

			std::unordered_map<CommandIdentifier, int, CIHash, CIEqual> m_commands;
			int m_commandState;
	};
}
