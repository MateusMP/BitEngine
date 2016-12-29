#pragma once

#include <unordered_map>

#include "bitengine/Core/System.h"
#include "bitengine/Core/Input.h"

namespace BitEngine {

	class InputSystem
		: public System
	{
		public:
			InputSystem(GameEngine* ge) : System(ge) {}
			virtual ~InputSystem() {}

			const char* getName() const override {
				return "Input";
			}

			virtual bool Init() = 0;
			virtual void Shutdown() = 0;
			virtual void Update() = 0;

			virtual Input::KeyMod isKeyPressed(int key) = 0;
			virtual Input::KeyMod keyReleased(int key) = 0;

			virtual double getMouseX() const = 0;
			virtual double getMouseY() const = 0;
			
	};
}
