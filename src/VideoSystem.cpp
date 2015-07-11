
#include "VideoSystem.h"
#include "Window.h"
#include "MessageChannel.h"
#include "MessageType.h"

#include <stdio.h>

namespace BitEngine{


void VideoSystem::GlfwFrameResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

VideoSystem::VideoSystem()
	: System("Video"), m_glfwWindow(nullptr)
{
    m_Window.m_Title = "WINDOW";
    m_Window.m_Width = 1280;
    m_Window.m_Height = 720;
    m_Window.m_Resizable = GL_TRUE;

    m_Window.m_RedBits = 8;
    m_Window.m_GreenBits = 8;
    m_Window.m_BlueBits = 8;
    m_Window.m_AlphaBits = 8;

    m_Window.m_DepthBits = 8;
    m_Window.m_StencilBits = 8;

}

VideoSystem::~VideoSystem(){
}

void VideoSystem::Shutdown()
{
    glfwTerminate();
}

bool VideoSystem::Init()
{
    printf("Init video...\n");
    if (!glfwInit()){
        return false;
    }

    printf("Creating window...\n");
    if (!CreateGLFWWindow()){
        glfwTerminate();
        return false;
    }
    printf("Video initialized!\n");

    glewExperimental = GL_TRUE;
    glewInit();

	glEnable(GL_TEXTURE_2D);

    Channel::Broadcast<WindowCreated>(WindowCreated(m_glfwWindow));

    return true;
}

bool VideoSystem::CreateGLFWWindow()
{
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, m_Window.m_Resizable);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);

    glfwWindowHint(GLFW_RED_BITS, m_Window.m_RedBits);
    glfwWindowHint(GLFW_GREEN_BITS, m_Window.m_GreenBits);
    glfwWindowHint(GLFW_BLUE_BITS, m_Window.m_BlueBits);
    glfwWindowHint(GLFW_ALPHA_BITS, m_Window.m_AlphaBits);
    glfwWindowHint(GLFW_DEPTH_BITS, m_Window.m_DepthBits);
    glfwWindowHint(GLFW_STENCIL_BITS, m_Window.m_StencilBits);

    m_glfwWindow = glfwCreateWindow(m_Window.m_Width, m_Window.m_Height, m_Window.m_Title.c_str(), NULL, NULL);
    if (!m_glfwWindow)
    {
        return false;
    }

    printf("Setting callback...\n");
    glfwSetFramebufferSizeCallback(m_glfwWindow, GlfwFrameResizeCallback);

    glfwMakeContextCurrent(m_glfwWindow);
    glfwSwapInterval(1);
    glfwShowWindow(m_glfwWindow);


    return true;
}

void VideoSystem::Update()
{
    if (glfwWindowShouldClose(m_glfwWindow)){
        Channel::Broadcast<WindowClose>(WindowClose());
        return;
    }

    glfwSwapBuffers(m_glfwWindow);
    glfwPollEvents();
}



}
