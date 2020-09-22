#pragma once

#include "Platform/glfw/GLFW_Headers.h"

#include "BitEngine/Core/Messenger.h"
#include "BitEngine/Core/Window.h"

namespace BitEngine {

class GLFW_ImGuiSystem {
public:
    GLFW_ImGuiSystem() = default;
    ~GLFW_ImGuiSystem() = default;

    /**
    * Initializes a window and openGL related stuff (Extensions and functions)
    * Currently using GLFW and GLEW
    */
    bool setup(BitEngine::Window* window);
    void update();

    void* getContext()
    {
        return context;
    }

    Messenger<ImGuiRenderEvent> events;

private:
    void* context = nullptr;
};
}
