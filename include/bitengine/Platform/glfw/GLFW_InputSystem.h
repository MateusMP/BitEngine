#pragma once

#include "bitengine/Core/InputSystem.h"
#include "bitengine/Platform/glfw/GLFW_Headers.h"

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

	void registerWindow(GLFWwindow* glfwWindow);
	void unregisterWindow(GLFWwindow* glfwWindow);
};