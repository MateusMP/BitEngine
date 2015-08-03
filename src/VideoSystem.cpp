
#include "VideoSystem.h"
#include "Window.h"
#include "MessageChannel.h"
#include "MessageType.h"

#include "EngineLoggers.h"

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
	LOGTO(Verbose) << "Video: Init video..." << endlog;
    if (!glfwInit()){
		LOGTO(Error) << "Video: Failed to initialize glfw!" << endlog;
        return false;
    }

	LOGTO(Verbose) << "Video: Creating window..." << endlog;
    if (!CreateGLFWWindow()){
		LOGTO(Error) << "Video: Failed to initialize video!" << endlog;
        glfwTerminate();
        return false;
    }
	LOGTO(Verbose) << "Video initialized!" << endlog;

    glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK){
		LOGTO(Error) << "Video: Failed to initialize opengl!" << endlog;
		glfwTerminate();
		return false;
	}

	// Log to error to force output on all build versions
	LOGTO(Error) << "[video info] Vendor: " << glGetString(GL_VENDOR) << endlog;
	LOGTO(Error) << "[video info] Renderer: " << glGetString(GL_RENDERER) << endlog;
	LOGTO(Error) << "[video info] Version: " << glGetString(GL_VERSION) << endlog;

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
		LOGTO(Error) << "Failed to create window!" << endlog;
        return false;
    }

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

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



}
