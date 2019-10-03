#pragma once

#include <unordered_map>

#include "bitengine/Core/System.h"
#include "bitengine/Core/Input.h"

namespace BitEngine {

	class InputSystem
		: public MessengerEndpoint
	{
		public:
			InputSystem(Messenger* m) : MessengerEndpoint(m) {}
			virtual ~InputSystem() {}
			
			virtual bool init() = 0;
			virtual void shutdown() = 0;
			virtual void update() = 0;

			virtual Input::KeyMod isKeyPressed(int key) = 0;
			virtual Input::KeyMod keyReleased(int key) = 0;

			virtual double getMouseX() const = 0;
			virtual double getMouseY() const = 0;
						
	};
}
