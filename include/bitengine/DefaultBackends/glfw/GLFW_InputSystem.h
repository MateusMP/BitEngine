#pragma once

#include "Core/InputSystem.h"

class GLFW_InputSystem : public BitEngine::InputSystem
{
public:
	GLFW_InputSystem(BitEngine::GameEngine* m);
	~GLFW_InputSystem(){}

	bool Init() override;
	void Shutdown() override;
	void Update() override;
	
	BitEngine::Input::KeyMod isKeyPressed(int key) override;
	BitEngine::Input::KeyMod keyReleased(int key) override;

	double getMouseX() const override;
	double getMouseY() const override;

	void onMessage(const BitEngine::MsgWindowCreated& wndcr);
	void onMessage(const BitEngine::MsgWindowClosed& wndcr);
};