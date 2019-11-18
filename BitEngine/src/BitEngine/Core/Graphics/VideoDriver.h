#pragma once

#include <memory>

#include "BitEngine/Core/EngineConfiguration.h"
#include "BitEngine/Core/Graphics/VideoRenderer.h"


namespace BitEngine {

	class Material;
	class RenderBuffer;
		
	class VideoDriver
	{
	public:
		VideoDriver();
		~VideoDriver();

		bool init();
		void shutdown();
		
		VideoAdapterType getVideoAdapter() const;

		/**
		* Clear buffer on defined options
		* \param buffer Buffer reference, if null, applies to screen
		* \param mask Options to clear, see BufferClearBitMask
		*/
		void clearBuffer(RenderBuffer* buffer, BufferClearBitMask mask);

		/**
		* If buffer == nullptr, clears the screen
		* otherwise, clears the given RenderBuffer
		**/
		void clearBufferColor(RenderBuffer* buffer, const ColorRGBA& color);

		void setViewPort(int x, int y, int width, int height);

		void configure(const Material* material);
	};

}