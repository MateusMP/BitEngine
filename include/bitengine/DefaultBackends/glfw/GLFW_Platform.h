#pragma once

#include "bitengine/DefaultBackends/glfw/GLFW_InputSystem.h"
#include "bitengine/DefaultBackends/glfw/GLFW_VideoSystem.h"

class GLFW_Platform : public BitEngine::MessengerEndpoint {
public:
	GLFW_Platform(BitEngine::Messenger* m)
		: MessengerEndpoint(m), video(m), input(m)
	{
		subscribe<GLFWWindowClosedMsg>(&GLFW_Platform::onMessage, this);
	}

	void onMessage(const GLFWWindowClosedMsg& msg) {
		if (m_currentWindow == msg.window) {
			m_currentWindow = nullptr;
			//getMessenger()->enqueue<UserQuitGame>(UserQuitGame());
		}
	}

	void init(BitEngine::EngineConfiguration& configuration) {
		video.init();
		input.init();

		BitEngine::WindowConfiguration windowConfig;

		windowConfig.m_Title = "WINDOW";
		windowConfig.m_Width = (u32) configuration.getConfiguration("Video", "Width", "1280")->getValueAsReal();
		windowConfig.m_Height = (u32) configuration.getConfiguration("Video", "Height", "720")->getValueAsReal();
		windowConfig.m_Resizable = configuration.getConfiguration("Video", "Resizable", "true")->getValueAsBool();
		windowConfig.m_FullScreen = configuration.getConfiguration("Video", "Fullscreen", "false")->getValueAsBool();

		windowConfig.m_RedBits = 8;
		windowConfig.m_GreenBits = 8;
		windowConfig.m_BlueBits = 8;
		windowConfig.m_AlphaBits = 8;

		windowConfig.m_DepthBits = 8;
		windowConfig.m_StencilBits = 8;

		m_currentWindow = video.createWindow(windowConfig);
		input.registerWindow(m_currentWindow);
	}

	void shutdown() {
		video.shutdown();
		input.shutdown();
	}

	GLFW_VideoSystem video;
	GLFW_InputSystem input;

	GLFWwindow* m_currentWindow;
};