#pragma once

#include "bitengine/Core/Resources/ResourceManager.h"

namespace BitEngine
{
	class Texture : public BaseResource
	{
		public:
			Texture(ResourceMeta* meta) : BaseResource(meta) {}
			virtual ~Texture() {}
			virtual u32 getTextureID() const = 0;
			virtual u32 getTextureType() const = 0;
	};
	using RTexture = RR<Texture>;
}
