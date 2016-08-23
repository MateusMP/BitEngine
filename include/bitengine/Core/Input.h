#pragma once

#include <unordered_map>

#include "Core/Messenger.h"
#include "Core/MessageType.h"

namespace BitEngine {
	namespace Input {

		enum class KeyMod : unsigned char {
			NONE = 0x0,
			KFALSE = 0x0,

			SHIFT = 0x0001,
			CTRL = 0x0002,
			ALT = 0x0004,
			SUPER = 0x0008,

			ALT_SHIFT = ALT | SHIFT,
			CTRL_ALT = CTRL | ALT,
			CTRL_SHIFT = CTRL | SHIFT,
			SUPER_ALT = SUPER | ALT,
			SUPER_CTRL = SUPER | CTRL,
			SUPER_SHIFT = SUPER | SHIFT,

			ALT_SHIFT_SUPER = ALT | SHIFT | SUPER,
			SHIFT_CTRL_SUPER = SHIFT | CTRL | SUPER,
			CTRL_ALT_SHIFT = CTRL | ALT | SHIFT,
			CTRL_ALT_SUPER = CTRL | ALT | SUPER,

			KTRUE = 0x0080  // Used to find out the current state of the key_press/release/ispressed events

							// Example: KeyPress/Release with no modifiers
							// 1000 0000 => x = KTRUE
							// Example: KeyPress/Release with CTRL+ALT
							// 1000 0110 => x = KTRUE | CTRL | ALT

							// Receiving the value:
							// Ex1:
							// if (x & KeyMod::KTRUE) --> Doesn't care for modifiers
							//
							// Ex2:
							// if (x & (KeyMod::KTRUE | KeyMod::CTRL | KeyMod::ALT) ) --> Check for key + CTRL + ALT
		};

		enum class KeyAction {
			NONE = 0x0,
			RELEASE = 0x1,
			PRESS = 0x2,
			REPEAT = 0x4,

			BASIC = RELEASE | PRESS,
			ALL = RELEASE | PRESS | REPEAT
		};

		enum class MouseAction {
			NONE = 0x0,
			RELEASE = 0x1,
			PRESS = 0x2,
			MOVE = 0x4,

		};

		// Message
		struct MsgKeyboardInput
		{
			MsgKeyboardInput()
				: key(0), keyAction(KeyAction::NONE), keyMod(KeyMod::KFALSE) {}
			MsgKeyboardInput(int k, KeyAction ka, KeyMod km)
				: key(k), keyAction(ka), keyMod(km) {}

			int key;
			KeyAction keyAction;
			KeyMod keyMod;
		};

		struct MsgMouseInput
		{
			MsgMouseInput()
				: button(0), action(MouseAction::NONE), keyMod(KeyMod::KFALSE), x(0), y(0) {}
			MsgMouseInput(double _x, double _y)
				: button(0), action(MouseAction::MOVE), keyMod(KeyMod::KFALSE), x(_x), y(_y) {}
			MsgMouseInput(int b, MouseAction ma, KeyMod km, double _x, double _y)
				: button(b), action(ma), keyMod(km), x(_x), y(_y) {}


			int button;
			MouseAction action;
			KeyMod keyMod;

			double x;
			double y;
		};

		class InputReceiver : public Messaging::MessengerEndpoint
		{
			public:
			InputReceiver(Messenger* m)
				: MessengerEndpoint(m), cursorInScreenX(0.0), cursorInScreenY(0.0)
			{}

			void keyboardInput(int key, int scancode, KeyAction action, int mods);
			void mouseInput(int button, MouseAction action, int mods);
			void mouseInput(double x, double y);

			KeyMod isKeyPressed(int key);
			KeyMod keyReleased(int key);

			double getMouseX() const;
			double getMouseY() const;

			private:
				std::unordered_map<unsigned int, KeyMod> m_keyDown;
				std::unordered_map<unsigned int, KeyMod> m_keyReleased;

				std::unordered_map<unsigned int, KeyMod> m_mouseDown;
				std::unordered_map<unsigned int, KeyMod> m_mouseReleased;

				double cursorInScreenX;
				double cursorInScreenY;
		};

		class InputDriver : public Messaging::MessengerEndpoint
		{
			public:
			InputDriver(Messenger* m) : MessengerEndpoint(m) {}
			virtual ~InputDriver() {}

			virtual bool Init() = 0;

			virtual void inputWindowCreated(BitEngine::Window* window) = 0;
			virtual void inputWindowDestroyed(BitEngine::Window* window) = 0;

			virtual KeyMod isKeyPressed(int key) = 0;
			virtual KeyMod keyReleased(int key) = 0;

			virtual double getMouseX() const = 0;
			virtual double getMouseY() const = 0;

			virtual void poolEvents() = 0;
		};

	}
}
