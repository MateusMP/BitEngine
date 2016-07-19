#pragma once

#include "Core/Resources/ResourceManager.h"

namespace BitEngine
{
	class ITexture : protected BaseResource
	{
		public:
			virtual uint32 getTextureID() const = 0;
			virtual uint32 getTextureType() const = 0;
	};
}
