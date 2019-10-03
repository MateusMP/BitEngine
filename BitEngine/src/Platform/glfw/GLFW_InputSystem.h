#pragma once

#include "bitengine/Core/InputSystem.h"
#include "Platform/glfw/GLFW_Headers.h"

namespace BitEngine {

class GLFW_InputSystem : public BitEngine::InputSystem
{
public:
	GLFW_InputSystem(BitEngine::Messenger* m);
	~GLFW_InputSystem(){}

	bool init() override;
	void shutdown() override;
	void update() override;
	
	BitEngine::Input::KeyMod isKeyPressed(int key) override;
	BitEngine::Input::KeyMod keyReleased(int key) override;

	double getMouseX() const override;
	double getMouseY() const override;

	void registerWindow(Window* glfwWindow);
	void unregisterWindow(Window* glfwWindow);
};

}