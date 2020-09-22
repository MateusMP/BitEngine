#pragma once

#include <vector>
#include "GLFW_Headers.h"
#include "BitEngine/Core/VideoSystem.h"
#include "Platform/video/VideoRenderer.h"

namespace BitEngine {

class GLFW_Window : public Window {
public:
    GLFW_Window(GLFWwindow* w)
        : window(w)
    {
    }
    virtual ~GLFW_Window();

    virtual void drawBegin() override;
    virtual void drawEnd() override;

private:
    friend class GLFW_ImGuiSystem;
    friend class GLFW_VideoSystem;
    friend class GLFW_InputSystem;
    GLFWwindow* window;
};

class GLFW_VideoSystem : public VideoSystem {
public:
    GLFW_VideoSystem()
        : glFuncsLoaded(false)
    {
    }
    ~GLFW_VideoSystem()
    {
        shutdown();
    }

    /**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
    bool init() override;
    void update() override;
    void shutdown() override;

    virtual u32 getVideoAdapter() override
    {
        return OPENGL_2;
    }

    virtual Window* createWindow(const WindowConfiguration& wc) override;

    virtual void closeWindow(Window* window) override;

protected:
private:
    bool glFuncsLoaded;
    std::vector<GLFW_Window*> m_windows;

    GLFW_Window* createGLFWWindow(const WindowConfiguration& wndConf);

    static void GLFW_ErrorCallback(int error, const char* description);

    LOG_CLASS(GLFW_VideoSystem, EngineLog);
};

} // namespace BitEngine