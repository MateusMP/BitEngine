#pragma once

#include <string>

namespace BitEngine{

	class Window
	{
	public:
		bool m_Resizable;
		bool m_FullScreen;

		unsigned int m_Width;
		unsigned int m_Height;

		unsigned int m_RedBits;
		unsigned int m_GreenBits;
		unsigned int m_BlueBits;
		unsigned int m_AlphaBits;

		unsigned int m_DepthBits;
		unsigned int m_StencilBits;

		std::string m_Title;
	};
}

#include "Graphics.h"
namespace BitEngine{

	class Window_glfw : public Window
	{
	public:
		Window_glfw()
			: m_glfwWindow(nullptr)
		{
		}

		GLFWwindow* m_glfwWindow;
	};

}

