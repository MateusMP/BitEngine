#pragma once

#include <string>

#include <bitengine/Common/TypeDefinition.h>

namespace BitEngine{

	class WindowConfiguration
	{
		public:
			bool m_Resizable;
			bool m_FullScreen;

			u32 m_Width;
			u32 m_Height;

			u32 m_RedBits;
			u32 m_GreenBits;
			u32 m_BlueBits;
			u32 m_AlphaBits;

			u32 m_DepthBits;
			u32 m_StencilBits;

			std::string m_Title;
	};
}


