#pragma once

#include <string>

namespace BitEngine{

	class WindowConfiguration
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


	struct MsgWindowClosed
	{
		MsgWindowClosed(Window *w) :window(w)
		{}

		Window* window;
	};

	struct MsgWindowCreated
	{
		MsgWindowCreated(Window *w) : window(w)
		{}

		Window* window;
	};
}


