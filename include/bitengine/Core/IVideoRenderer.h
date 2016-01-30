#pragma once

namespace BitEngine {

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
		COLOR,
		DEPTH,
	};

	class IRenderBuffer
	{
		virtual void Unbind() = 0;
		virtual void Bind() = 0;
		virtual void BindDraw() = 0;
		virtual void BindRead() = 0;
		virtual bool Ready() = 0;
	};

	class IVideoRenderer
	{
		public:

			/**
			 * Clear buffer on defined options
			 * \param buffer Buffer reference, if null, applies to screen
			 * \param mask Options to clear, see BufferClearBitMask
			 */
			virtual void ClearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask) = 0;

			/**
			* If buffer == nullptr, clears the screen
			* otherwise, clears the given RenderBuffer
			**/
			virtual void ClearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color) = 0;

			virtual void setViewPort(int x, int y, int width, int height) = 0;
	};


}