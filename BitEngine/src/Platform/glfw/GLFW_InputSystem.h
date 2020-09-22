#pragma once

#include "BitEngine/Core/InputSystem.h"
#include "Platform/glfw/GLFW_Headers.h"

namespace BitEngine {

class GLFW_InputSystem : public BitEngine::InputSystem {
public:
    GLFW_InputSystem();
    ~GLFW_InputSystem() {}

    bool init() override;
    void shutdown() override;
    void update() override;

    KeyMod isKeyPressed(int key) override;
    KeyMod keyReleased(int key) override;

    double getMouseX() const override;
    double getMouseY() const override;

    void registerWindow(Window* glfwWindow);
    void unregisterWindow(Window* glfwWindow);
};
}