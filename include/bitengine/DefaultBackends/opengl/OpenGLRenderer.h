#pragma once

#include "OpenGLheaders.h"

#include "Core/IVideoRenderer.h"

namespace BitEngine {

	class OpenGLRenderer : public IVideoRenderer
	{
	public:
		~OpenGLRenderer() {}

		void ClearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask) override
		{
			GLbitfield bitfield = 0;
			if (mask & BufferClearBitMask::COLOR)
				bitfield |= GL_COLOR_BUFFER_BIT;
			if (mask & BufferClearBitMask::DEPTH)
				bitfield |= GL_DEPTH_BUFFER_BIT;

			glClear(bitfield);
		}

		void ClearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color) override
		{
			glClearColor(color.r(), color.g(), color.b(), color.a());
		}

		void setViewPort(int x, int y, int width, int height) override
		{
			glViewport(x, y, width, height);
		}
	};

}
