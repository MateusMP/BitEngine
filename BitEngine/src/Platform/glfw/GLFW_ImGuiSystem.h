#pragma once

#include <GLFW/glfw3.h>

#include "bitengine/Core/Messenger.h"
#include "BitEngine/Core/Window.h"

namespace BitEngine {

class GLFW_ImGuiSystem : public Messenger<ImGuiRenderEvent>
{
public:
    GLFW_ImGuiSystem()
    {
    }
    ~GLFW_ImGuiSystem() {}

    /**
    * Initializes a window and openGL related stuff (Extensions and functions)
    * Currently using GLFW and GLEW
    */
    bool setup(BitEngine::Window* window);
    void update();

    void* getContext() {
        return context;
    }
private:
    void* context;
};

}
