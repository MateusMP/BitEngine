#pragma once

#include "Graphics.h"
#include "System.h"
#include "Window.h"

#include <set>

namespace BitEngine{

	template<unsigned channels, unsigned R, unsigned G, unsigned B, unsigned A>
	struct Color {
		enum Channel { RED = R, GREEN = G, BLUE = B, ALPHA = A, CHANNELS = channels };
		float& operator [] (Channel x) {
			return colors[x];
		}
		inline float r() const { return colors[R]; }
		inline float g() const { return colors[G]; }
		inline float b() const { return colors[B]; }
		inline float a() const { return colors[A]; }

		private:
			float colors[channels];
	};
	typedef Color<4, 0, 1, 2, 3> ColorRGBA;
	typedef Color<3, 0, 1, 2, 3> ColorRGB;

	enum BufferClearBitMask {
		COLOR,
		DEPTH,
	};

	class IRenderBuffer
	{
	};

	class IRenderer
	{
		public:
			
			virtual void ClearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask);

			/**
			 * If buffer == nullptr, clears the screen
			 * otherwise, clears the given RenderBuffer
			 **/
			virtual void ClearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color);

			virtual void SetViewPort(int x, int y, int width, int height);
	};

	class OpenGLRenderer : public IRenderer
	{
		void ClearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask) override {
			GLbitfield bitfield;
			if (mask & BufferClearBitMask::COLOR)
				bitfield |= GL_COLOR_BUFFER_BIT;
			if (mask & BufferClearBitMask::DEPTH)
				bitfield |= GL_DEPTH_BUFFER_BIT;

			glClear(bitfield);
		}

		void ClearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color) override {
			glClearColor(color.r(), color.g(), color.b(), color.a());
		}

		void SetViewPort(int x, int y, int width, int height) override {
			glViewport(x, y, width, height);
		}
	};

	/**
	 * Override this class for creating your own rendering pipeline
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem(const std::string& name) : System(name) {}

			virtual ~VideoSystem(){}
		
			/**
			 * Initializes a window and it's rendering driver
			 */
			virtual bool Init() = 0;

			/**
			 * Close the Video System
			 */
			virtual void Shutdown() = 0;

			/**
			 * Called once a frame
			 */
			virtual void Update() = 0;

			/**
			 * Called when a window is resized
			 */
			virtual void OnWindowResize(Window* window, int width, int height) = 0;
	};

}


