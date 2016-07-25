#pragma once

#include <unordered_map>

#include "Core/System.h"
#include "Core/Input.h"

namespace BitEngine {

	class InputSystem
		: public System
	{
		public:
		InputSystem(GameEngine* ge, Input::IInputDriver *input);
		~InputSystem();

		const char* getName() const override {
			return "Input";
		}

		bool Init() override;
		void Shutdown() override;
		void Update() override;

		void onMessage(const MsgWindowCreated& wndcr);
		void onMessage(const MsgWindowClosed& wndcr);

		private:
		Input::IInputDriver *driver;
	};
}