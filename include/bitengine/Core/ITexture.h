#pragma once

#include "IResourceManager.h"

namespace BitEngine
{

	class ITexture : protected BaseResource
	{
		public:
			virtual uint32 getTextureID() const = 0;
			virtual uint32 getTextureType() const = 0;
	};

	class ITextureManager : public IResourceManager
	{
		public:
		virtual void Update() = 0;

		// Always returns a valid pointer.
		// The texture might be invalid (a default texture) until the loader finishes loading it
		virtual ITexture* getTexture(const std::string& str) = 0;
		virtual ITexture* getTexture(uint32 id) = 0;

	};

}
