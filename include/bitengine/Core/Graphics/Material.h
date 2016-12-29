#pragma once

#include <cstring>

#include "bitengine/Core/Graphics/VideoRenderer.h"
#include "bitengine/Common/TypeDefinition.h"

namespace BitEngine {

	class Material {
		public:
		Material()
			: blendEquation(BlendEquation::ADD)
		{
			memset(states, 0, sizeof(states));
		}

		void setState(RenderConfig config, u8 mode) {
			states[(u8)config] = mode;
		}
		void setBlendMode(BlendFunc src, BlendFunc dst) {
			srcColorBlendMode = src;
			dstColorBlendMode = dst;
		}
		void setBlendModeSeparate(BlendFunc srcColor, BlendFunc dstColor, BlendFunc srcAlpha, BlendFunc dstAlpha) {
			srcColorBlendMode = srcColor;
			dstColorBlendMode = dstColor;
			srcAlphaBlendMode = srcAlpha;
			dstAlphaBlendMode = dstAlpha;
		}
		void setBlendEquation(BlendEquation eq) {
			blendEquation = eq;
		}

		u8 getState(RenderConfig config) const {
			return states[(u8)config];
		}

		u8 states[(u8)RenderConfig::TOTAL_RENDER_CONFIGS];
		BlendFunc srcColorBlendMode;
		BlendFunc srcAlphaBlendMode;
		BlendFunc dstColorBlendMode;
		BlendFunc dstAlphaBlendMode;
		BlendEquation blendEquation;
	};

}
