#pragma once

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

