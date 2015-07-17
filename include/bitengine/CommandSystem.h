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
				CommandInput(int _id, float _intensity);

				const int commandID;
				const float intensity;
				int mouse_x; // Invalid unless Mouse input was used
				int mouse_y; // Invalid unless Mouse input was used

				union {
					InputReceiver::KeyAction fromButton; // keyboard / mouse / joystick buttons
					int other;
				} other;

			};

			CommandSystem();
			~CommandSystem();

			bool Init() override;
			void Shutdown() override;
			void Update() override;
		
			void setCommandState(int state);

			/// \commandID the command ID returned when given key/action/mod combination ocurrs
			/// \param commandState CommandSystem state necessary for processing this command
			/// \key keyboard key needed
			/// Note that this function will register the commandID for both inputs: RELEASE and PRESS
			bool RegisterKeyboardCommand(int commandID, int commandState, int key);

			/// \commandID the command ID returned when given key/action/mod combination ocurrs
			/// \param commandState CommandSystem state necessary for processing this command
			/// \key keyboard key needed
			/// \action key state needed
			/// \mod key modifiers (Shift, Alt, Ctrl, Super)
			bool RegisterKeyboardCommand(int commandID, int commandState, int key, InputReceiver::KeyAction action, InputReceiver::KeyMod mod = InputReceiver::KeyMod::NONE);


			void Message(const InputReceiver::KeyboardInput& msg);

		private:
			enum class InputType : int{
				keyboard,
				mouse,
				joystick
			};

			struct CommandIdentifier{
				CommandIdentifier(int s, InputType it)
					: commandState(s), commandInputType(it){}
				int commandState;
				InputType commandInputType;
				
				InputReceiver::KeyboardInput keyboard;
			};
			
			// CommandIdentifier Hash and Equal
			class CIHash{
			public:
				std::size_t operator()(const CommandIdentifier& k) const
				{
					return (std::hash<int>()(k.commandState << 24) 
						 ^ (std::hash<int>()((int)k.commandInputType) << 16))
						 ^ (std::hash<int>()(k.keyboard.key) << 8)
						 ^ (std::hash<int>()((int)k.keyboard.keyAction) << 4)
						 ^ (std::hash<int>()((int)k.keyboard.keyMod));
				}
			};
			class CIEqual{
			public:
				bool operator() (const CommandIdentifier& t1, const CommandIdentifier& t2) const
				{
					return (t1.commandState == t2.commandState
						&& t1.commandInputType == t2.commandInputType
						&& t1.keyboard.key == t2.keyboard.key
						&& t1.keyboard.keyAction == t2.keyboard.keyAction
						&& t1.keyboard.keyMod == t2.keyboard.keyMod);

				}
			};

			std::unordered_map<CommandIdentifier, int, CIHash, CIEqual> m_commands;
			int m_commandState;

	};

}