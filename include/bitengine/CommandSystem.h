#pragma once

#include "System.h"
#include "InputSystem.h"

#include <unordered_map>

#include "MessageChannel.h"

namespace BitEngine{

	class CommandSystem : public System {
		public:
			struct CommandInput{
				CommandInput(int _id, float _intensity, int _other);
				CommandInput(int _id, float _intensity, InputReceiver::KeyAction _other);
				CommandInput(int _id, float _intensity, InputReceiver::MouseAction _other, double x, double y);
				CommandInput(int _id, float _intensity);

				const int commandID;
				const float intensity;
				double mouse_x; // Invalid unless Mouse input was used
				double mouse_y; // Invalid unless Mouse input was used

				union {
					InputReceiver::KeyAction fromButton; // keyboard / mouse / joystick buttons
					InputReceiver::MouseAction fromMouse;
					int other;
				} other;

			};

			CommandSystem();
			~CommandSystem();

			bool Init() override;
			void Shutdown() override;
			void Update() override;
		
			void setCommandState(int state);

			/** @param commandID	the command ID returned when given key/action/mod combination ocurrs
			  * @param commandState		CommandSystem state necessary for processing this command
			  *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			  * @param key	keyboard key needed
			  * Note that this function will register the commandID for both inputs: RELEASE and PRESS
			  */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key);

			/** @param commandID	Command ID returned when given key/action/mod combination ocurrs
			  * @param commandState		CommandSystem state necessary for processing this command
			  *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
			  * @param key	Keyboard key needed
			  * @param action	Key state needed
			  * @param mod	Key modifiers (Shift, Alt, Ctrl, Super)
			  */
			bool RegisterKeyboardCommand(int commandID, int commandState, int key, InputReceiver::KeyAction action, InputReceiver::KeyMod mod = InputReceiver::KeyMod::NONE);

			bool RegisterMouseCommand(int commandID, int commandState, int button, InputReceiver::MouseAction action, InputReceiver::KeyMod mod = InputReceiver::KeyMod::NONE);

			void Message(const InputReceiver::KeyboardInput& msg);
			void Message(const InputReceiver::MouseInput& msg);

		private:
			enum class InputType : int{
				keyboard,
				mouse,
				joystick
			};

			struct CommandIdentifier{
				CommandIdentifier()
				{}
				CommandIdentifier(int s, const InputReceiver::KeyboardInput& k)
					: commandState(s), commandInputType(InputType::keyboard), keyboard(k){}
				CommandIdentifier(int s, const InputReceiver::MouseInput& m)
					: commandState(s), commandInputType(InputType::mouse), mouse(m){}
				int commandState;
				InputType commandInputType;
				
				InputReceiver::KeyboardInput keyboard;
				InputReceiver::MouseInput mouse;
			};
			
			// CommandIdentifier Hash and Equal
			class CIHash{
			public:
				std::size_t operator()(const CommandIdentifier& k) const
				{
					if (k.commandInputType == InputType::keyboard)
						return (std::hash<int>()(k.commandState << 24) 
							 ^ (std::hash<int>()((int)k.commandInputType) << 16))
							 ^ (std::hash<int>()(k.keyboard.key) << 8)
							 ^ (std::hash<int>()((int)k.keyboard.keyAction) << 4)
							 ^ (std::hash<int>()((int)k.keyboard.keyMod));
					else if (k.commandInputType == InputType::mouse)
						return (std::hash<int>()(k.commandState << 24)
							^ (std::hash<int>()((int)k.commandInputType) << 16))
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
					if (t1.commandInputType == InputType::keyboard && t2.commandInputType == InputType::keyboard)
						return (t1.commandState == t2.commandState
							&& t1.keyboard.key == t2.keyboard.key
							&& t1.keyboard.keyAction == t2.keyboard.keyAction
							&& t1.keyboard.keyMod == t2.keyboard.keyMod);
					else if (t1.commandInputType == InputType::mouse && t2.commandInputType == InputType::mouse)
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