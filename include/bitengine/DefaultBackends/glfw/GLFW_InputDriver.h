#pragma once

#include "Core/InputSystem.h"

class GLFW_InputDriver : public BitEngine::Input::InputDriver
{
public:
	GLFW_InputDriver(BitEngine::Messenger* m);
	~GLFW_InputDriver(){}

	bool Init() override;

	void inputWindowCreated(BitEngine::Window* window) override;
	void inputWindowDestroyed(BitEngine::Window* window) override;

	BitEngine::Input::KeyMod isKeyPressed(int key) override;

	BitEngine::Input::KeyMod keyReleased(int key) override;

	double getMouseX() const override;

	double getMouseY() const override;

	void poolEvents() override;
};