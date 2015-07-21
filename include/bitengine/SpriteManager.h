#pragma once

#include <vector>
#include <map>

#include "Sprite.h"

namespace BitEngine{

	class SpriteManager{
	public:
		SpriteManager();

		SpriteHandle createSprite(const std::string& name, const Sprite& spr);

		/// This sprite wont be avaible through getSprite(name)
		SpriteHandle createSprite(const Sprite& spr);

		void removeSprite(SpriteHandle hdl);

		SpriteHandle getSpriteHandle(const std::string& name);

		const Sprite* getSprite(SpriteHandle hdl);

	private:
		std::map<std::string, SpriteHandle> m_sprLookUp;
		std::map<SpriteHandle, std::string> m_sprLookUpInv;
		std::vector<Sprite> m_sprites;
		std::vector<SpriteHandle> m_freeSlots;
		
	};


}