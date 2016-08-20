#pragma once

#include "Core/VideoRenderer.h"
#include "Common/TypeDefinition.h"

namespace BitEngine {

	class Material {

		void setState(RenderConfig config, u8 mode) {
			states[config] = mode;
		}
		void setBlendMode(BlendMode src, BlendMode dst) {
			srcColorBlendMode = src;
			dstColorBlendMode = dst;
		}
		void setBlendModeSeparate(BlendMode srcColor, BlendMode dstColor, BlendMode srcAlpha, BlendMode dstAlpha) {
			srcColorBlendMode = srcColor;
			dstColorBlendMode = dstColor;
			srcAlphaBlendMode = srcAlpha;
			dstAlphaBlendMode = dstAlpha;
		}
		void setBlendEquation(BlendEquation eq) {
			blendEquation = eq;
		}

		private:
		u8 states[RenderConfig::TOTAL_RENDER_CONFIGS];
		BlendMode srcColorBlendMode;
		BlendMode srcAlphaBlendMode;
		BlendMode dstColorBlendMode;
		BlendMode dstAlphaBlendMode;
		BlendEquation blendEquation;
	};

}