#pragma once

#include "Graphics/Shader.h"

namespace BitEngine {

	enum PipelineType{
		NONE,
		OPENGL_2,
		OPENGL_4,

		VULKAN_1,

		GL_ALL,
		GL_2_OR_GREATER,
		GL_4_OR_GREATER,

		VULKAN_ALL,
	};

	template<unsigned channels, unsigned R, unsigned G, unsigned B, unsigned A>
	struct Color
	{
		static_assert(R != G && G != B && B != A, "Invalid color index");

		public:
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

	enum BufferClearBitMask
	{
		COLOR = 1,
		DEPTH = 2,

		COLOR_DEPTH = COLOR | DEPTH
	};

	class IRenderBuffer
	{
		virtual void Unbind() = 0;
		virtual void Bind() = 0;
		virtual void BindDraw() = 0;
		virtual void BindRead() = 0;
		virtual bool Ready() = 0;
	};

}
