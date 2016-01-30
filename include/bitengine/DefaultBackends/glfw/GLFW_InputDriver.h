#pragma once

#include <unordered_map>

#include "Core/MessageType.h"
#include "Core/InputSystem.h"
#include "Core/Logger.h"

#include "GLFW_VideoDriver.h"

class GLFW_InputDriver : public BitEngine::IInputDriver
{
public:
	void inputWindowCreated(BitEngine::Window* window) override;
	void inputWindowDestroyed(BitEngine::Window* window) override;

	BitEngine::InputReceiver::KeyMod isKeyPressed(int key) override;

	BitEngine::InputReceiver::KeyMod keyReleased(int key) override;

	double getMouseX() const override;

	double getMouseY() const override;

	void poolEvents() override;

private:
	static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void GlfwMouseCallback(GLFWwindow* window, int button, int action, int mods);
	static void GlfwMousePosCallback(GLFWwindow* window, double x, double y);

	static std::unordered_map<GLFWwindow*, BitEngine::InputReceiver> inputReceivers;

	LOG_STATIC("GLFW_InputDriver", BitEngine::EngineLog);
};