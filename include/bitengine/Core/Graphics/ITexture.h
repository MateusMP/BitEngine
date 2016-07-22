#pragma once

#include "Core/Resources/ResourceManager.h"

namespace BitEngine
{
	class ITexture : protected BaseResource
	{
		public:
			ITexture(ResourceMeta* meta) : BaseResource(meta) {}
			virtual uint32 getTextureID() const = 0;
			virtual uint32 getTextureType() const = 0;
	};
}
