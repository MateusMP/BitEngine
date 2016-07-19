#pragma once

#include <vector>
#include <map>

#include "Core/Resources/ResourceManager.h"
#include "Core/Sprite.h"

namespace BitEngine{
	
	class SpriteManager{
	public:
		SpriteManager(ResourceLoader* textureManager);

		bool Init();

		/**
		 * Insert a new sprite
		 * The name is used for name look-up, but all sprites can be found
		 * using the returned SpriteHandle.
		 * \param Name used for indexing
		 * \param Sprite data to be used
		 * \param return SpriteHandle for direct acess to given sprite
		 */
		SpriteHandle createSprite(const std::string& name, const Sprite& spr);

		/** 
		 * Insert a new sprite
		 * This sprite wont be avaible through getSprite(name)
		 */
		SpriteHandle createSprite(const Sprite& spr);

		/**
		 * Remove the sprite freeing the SpriteHandle to be used
		 * by another sprite.
		 */
		void removeSprite(SpriteHandle hdl);

		/**
		 * Replaces given SpriteHandle.
		 * The SpriteHandle should be valid for an existing sprite.
		 * The sprite name associated with it wont be changed.
		 * @returns false when the handle is invalid
		 */
		bool replaceSprite(SpriteHandle hdl, const Sprite& spr);

		/**
		 * \param name Sprite name to find the SpriteHandle
		 * @return Returns the SpriteHandle associated with the sprite name.
		 *			If not found, 0 is returned.
		 */
		SpriteHandle getSpriteHandle(const std::string& name) const;

		/**
		 * Get a reference for given sprite handle
		 * This reference is not guaranteed to be valid after the sprite structure is changed,
		 * like new sprites or removed sprites.
		 */
		const Sprite* getSprite(SpriteHandle hdl) const;

	private:
		std::map<std::string, SpriteHandle> m_sprLookUp;
		std::map<SpriteHandle, std::string> m_sprLookUpInv;
		std::vector<Sprite> m_sprites;
		std::vector<SpriteHandle> m_freeSlots;

		ResourceLoader* m_textureManager;
		
	};


}