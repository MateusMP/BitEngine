#pragma once

#include <unordered_map>

#include "Core/System.h"
#include "Core/InputSystem.h"
#include "Core/Message.h"

namespace BitEngine{

	class CommandSystem : public System
	{
		public:
			struct MsgCommandInput
			{
				public:
					MsgCommandInput(int _id, float _intensity, int _other);
					MsgCommandInput(int _id, float _intensity, Input::KeyAction _other);
					MsgCommandInput(int _id, float _intensity, Input::MouseAction _other, double x, double y);
					MsgCommandInput(int _id, float _intensity);

					const int commandID;
					const float intensity;
					double mouse_x; // Invalid unless Mouse input was used
					double mouse_y; // Invalid unless Mouse input was used

					union {
						Input::KeyAction fromButton; // keyboard / mouse / joystick buttons
						Input::MouseAction fromMouse;
						int other;
					} other;
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

			/** \param commandID	the command ID returned when given key/action/mod combination ocurrs
			  * \param commandState		CommandSystem state necessary for processing this command
			  *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			  * \param key	keyboard key needed
			  * Note that this function will register the commandID for both inputs: RELEASE and PRESS
			  */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key);

			/** \param commandID	the command ID returned when given key/action/mod combination ocurrs
			  * \param commandState		CommandSystem state necessary for processing this command
			  *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			  * \param key	keyboard key needed
			  * Note that this function will register the commandID for both inputs: RELEASE and PRESS and to be accepted with any keymod combination
			  * 
			  * If an ambiguous keybind is found, it wont be overwritten and the command being bind will not be fully binded.
			  * In this case, the function returns false.
			  * It's recommended that all commands for the requested commandID are cleared if this function fails. 
			  * Left optional for the caller, so no other binding will be silently removed.
			  */
			bool RegisterKeyCommandForAllMods(int commandID, int commandState, int key);

			/** \param commandID	Command ID returned when given key/action/mod combination ocurrs
			  * \param commandState		CommandSystem state necessary for processing this command
			  *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			  * \param key	Keyboard key needed
			  * \param action	Key state needed
			  * \param mod	Key modifiers (Shift, Alt, Ctrl, Super)
			  */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key, Input::KeyAction action, Input::KeyMod mod = Input::KeyMod::NONE);

			/**
			 * Same for mouse commands
			 */
			bool RegisterMouseCommand(int commandID, int commandState, int button, Input::MouseAction action, Input::KeyMod mod = Input::KeyMod::NONE);

			bool RegisterMouseMove(int commandID, int commandState, Input::KeyMod mod = Input::KeyMod::NONE){
				return RegisterMouseCommand(commandID, commandState, 0, Input::MouseAction::MOVE, mod);
			}

			void onMessage(const Input::MsgKeyboardInput& msg_);
			void onMessage(const Input::MsgMouseInput& msg);

		private:
			enum class InputType : char{
				keyboard,
				mouse,
				joystick
			};

			struct CommandIdentifier{
				CommandIdentifier()
				{}

				CommandIdentifier(int s, const Input::MsgKeyboardInput& k)
					: commandState(s), MsgCommandInputType(InputType::keyboard), keyboard(k){}

				CommandIdentifier(int s, const Input::MsgMouseInput& m)
					: commandState(s), MsgCommandInputType(InputType::mouse), mouse(m){}

				int commandState;
				InputType MsgCommandInputType;

				Input::MsgKeyboardInput keyboard;
				Input::MsgMouseInput mouse;
			};

			// CommandIdentifier Hash and Equal
			class CIHash{
			public:
				std::size_t operator()(const CommandIdentifier& k) const
				{
					if (k.MsgCommandInputType == InputType::keyboard)
						return (std::hash<int>()(k.commandState << 24)
							 ^ (std::hash<int>()((int)k.MsgCommandInputType) << 16))
							 ^ (std::hash<int>()(k.keyboard.key) << 8)
							 ^ (std::hash<int>()((int)k.keyboard.keyAction) << 4)
							 ^ (std::hash<int>()((int)k.keyboard.keyMod));
					else if (k.MsgCommandInputType == InputType::mouse)
						return (std::hash<int>()(k.commandState << 24)
							^ (std::hash<int>()((int)k.MsgCommandInputType) << 16))
							^ (std::hash<int>()(k.mouse.button) << 8)
							^ (std::hash<int>()((int)k.mouse.action) << 4)
							^ (std::hash<int>()((int)k.mouse.keyMod));

					return 0;
				}
			};
			class CIEqual{
			public:
				bool operator() (const CommandIdentifier& t1, const CommandIdentifier& t2) const
				{
					if (t1.MsgCommandInputType == InputType::keyboard && t2.MsgCommandInputType == InputType::keyboard)
						return (t1.commandState == t2.commandState
							&& t1.keyboard.key == t2.keyboard.key
							&& t1.keyboard.keyAction == t2.keyboard.keyAction
							&& t1.keyboard.keyMod == t2.keyboard.keyMod);
					else if (t1.MsgCommandInputType == InputType::mouse && t2.MsgCommandInputType == InputType::mouse)
						return (t1.commandState == t2.commandState
							&& t1.mouse.button == t2.mouse.button
							&& t1.mouse.action == t2.mouse.action
							&& t1.mouse.keyMod == t2.mouse.keyMod);

					return false;
				}
			};

			std::unordered_map<CommandIdentifier, int, CIHash, CIEqual> m_commands;
			int m_commandState;

	};

}
