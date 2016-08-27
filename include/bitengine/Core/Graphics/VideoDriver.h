#pragma once

#include "Core/GameEngine.h"
#include "Core/EngineConfiguration.h"
#include "Core/Graphics/VideoRenderer.h"

namespace BitEngine {

	class Material;
	class RenderBuffer;

	class VideoDriver : public EnginePiece
	{
	public:
		VideoDriver(GameEngine* ge) : EnginePiece(ge) {}
		virtual ~VideoDriver() {}

		virtual bool init() = 0;
		virtual void shutdown() = 0;
		
		virtual VideoAdapterType getVideoAdapter() const = 0;

		/**
		* Clear buffer on defined options
		* \param buffer Buffer reference, if null, applies to screen
		* \param mask Options to clear, see BufferClearBitMask
		*/
		virtual void clearBuffer(RenderBuffer* buffer, BufferClearBitMask mask) = 0;

		/**
		* If buffer == nullptr, clears the screen
		* otherwise, clears the given RenderBuffer
		**/
		virtual void clearBufferColor(RenderBuffer* buffer, const ColorRGBA& color) = 0;

		virtual void setViewPort(int x, int y, int width, int height) = 0;

		virtual void configure(const Material* material) = 0;
	};

}