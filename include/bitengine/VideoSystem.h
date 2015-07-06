#pragma once

#include "Graphics.h"
#include "System.h"
#include "Window.h"

namespace BitEngine{


class VideoSystem : public System
{
    public:
        VideoSystem();
        ~VideoSystem();

        const std::string& getName() const override;

        bool Init() override;
        void Shutdown() override;
        void Update() override;


    private:
        bool CreateGLFWWindow();

        Window m_Window;
        GLFWwindow* m_glfwWindow;

        static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);

        std::string m_name;
};

}


