#pragma once

#include "Core/GameEngine.h"
#include "Core/Graphics/VideoDriver.h"

#include "OpenGL2.h"

namespace BitEngine {

	class GL2Driver : public BitEngine::VideoDriver
	{
	public:
		GL2Driver(BitEngine::GameEngine* ge) : VideoDriver(ge) {}
		~GL2Driver() {}

		bool init() override;
		void shutdown() override;

		virtual BitEngine::VideoAdapterType getVideoAdapter() const override {
			return BitEngine::VideoAdapterType::GL_2_OR_LOWER;
		}

		/**
		* Clear buffer on defined options
		* \param buffer Buffer reference, if null, applies to screen
		* \param mask Options to clear, see BufferClearBitMask
		*/
		virtual void clearBuffer(BitEngine::RenderBuffer* buffer, BitEngine::BufferClearBitMask mask) override;

		/**
		* If buffer == nullptr, clears the screen
		* otherwise, clears the given RenderBuffer
		**/
		virtual void clearBufferColor(BitEngine::RenderBuffer* buffer, const BitEngine::ColorRGBA& color) override;

		virtual void setViewPort(int x, int y, int width, int height) override;

		virtual void configure(const BitEngine::Material* material) override;


	private:
		BitEngine::GL2ShaderManager* shaderManager;
		BitEngine::GL2TextureManager* textureManager;
	};
}