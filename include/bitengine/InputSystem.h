#pragma once

#include "Graphics.h"

#include "System.h"
#include "MessageChannel.h"
#include "MessageType.h"

#include <unordered_map>

namespace BitEngine{

class InputReceiver
{
	public:
		enum class KeyMod : unsigned char{
			NONE = 0x0,
			FALSE = 0x0,

			SHIFT = 0x0001,
			CTRL = 0x0002,
			ALT = 0x0004,
			SUPER = 0x0008,

			TRUE = 0x0080

			// Example: KeyPress/Release with no modifiers
			// 1000 0000 => x = TRUE
			// Example: KeyPress/Release with no CTRL+ALT
			// 1000 0110 => x = TRUE | CTRL | ALT

			// Receiving the value:
			// Ex1:
			// if (x & KeyMod::TRUE) --> Doesn't care for modifiers
			//
			// Ex2:
			// if (x & (KeyMod::TRUE | KeyMod::CTRL | KeyMod::ALT) ) --> Check for key + CTRL + ALT
		};

		enum class KeyAction{
			NONE = 0x0,
			RELEASE = 0x1,
			PRESS = 0x2,
			REPEAT = 0x4,

			BASIC = RELEASE | PRESS,
			ALL = RELEASE | PRESS | REPEAT
		};

		// Message
		struct KeyboardInput{
			KeyboardInput() : key(0), keyAction(KeyAction::NONE), keyMod(KeyMod::FALSE){}
			KeyboardInput(int k, KeyAction ka, KeyMod km)
			: key(k), keyAction(ka), keyMod(km){}

			int key;
			KeyAction keyAction;
			KeyMod keyMod;
		};

	public:
		void keyboardInput(int key, int scancode, int action, int mods);

		KeyMod isKeyPressed(int key);
		KeyMod keyReleased(int key);

	private:
		std::unordered_map<unsigned int, KeyMod> m_keyDown;
		std::unordered_map<unsigned int, KeyMod> m_keyReleased;
};

class InputSystem
    : public System
{
    public:
        InputSystem();
        ~InputSystem();
		
        bool Init() override;
        void Shutdown() override;
        void Update() override;

        void Message(const WindowCreated& wndcr);

		// Will verify for the main window (first created)
		InputReceiver::KeyMod isKeyPressed(int key);
		// Will verify for the main window (first created)
		InputReceiver::KeyMod keyReleased(int key);

    private:
        static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		
		static std::unordered_map<GLFWwindow*, InputReceiver> inputReceivers;

};


}

