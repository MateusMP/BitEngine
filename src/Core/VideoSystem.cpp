
#include <stdio.h>

#include "Core/VideoSystem.h"
#include "Core/Logger.h"

namespace BitEngine {

	bool VideoSystem::Init()
	{
		VideoConfiguration config;
		WindowConfiguration windowConfig;

		windowConfig.m_Title = "WINDOW";
		windowConfig.m_Width = 1280;
		windowConfig.m_Height = 720;
		windowConfig.m_Resizable = GL_TRUE;
		windowConfig.m_FullScreen = getConfig("Fullscreen", "false")->getValueAsBool();

		windowConfig.m_RedBits = 8;
		windowConfig.m_GreenBits = 8;
		windowConfig.m_BlueBits = 8;
		windowConfig.m_AlphaBits = 8;

		windowConfig.m_DepthBits = 8;
		windowConfig.m_StencilBits = 8;

		m_driver->setMessenger(getEngine()->getMessenger());

		if (m_driver->Init(config))
		{
			getEngine()->getResourceLoader()->registerResourceManager("TEXTURE", m_driver->getRenderer()->getTextureManager());
			getEngine()->getResourceLoader()->registerResourceManager("SHADER", m_driver->getRenderer()->getShaderManager());

			m_window = m_driver->CreateWindow(windowConfig);
			if (m_window)
			{

			}

			return m_window != nullptr;
		}

		return false;
	}

}