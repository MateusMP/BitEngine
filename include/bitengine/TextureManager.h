#pragma once

#include <map>

#include "Texture.h"

namespace BitEngine{

	class TextureManager
	{
		public:
			typedef std::map<std::string, Texture*> TextureMap;

			TextureManager();
			~TextureManager();

			bool Init();

			const Texture* getErrorTexture();
			const Texture* LoadTexture2D(const std::string& path);

		private:
			TextureMap m_textures;

			Texture error_texture;
	};



}