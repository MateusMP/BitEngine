
#include "SpriteManager.h"

#include "EngineLoggers.h"

namespace BitEngine{

	SpriteManager::SpriteManager(){
		// TODO: Create default invalid sprite texture
		m_sprites.emplace_back(); // First sprite is INVALID
	}

	SpriteHandle SpriteManager::createSprite(const std::string& name, const Sprite& spr)
	{
		auto it = m_sprLookUp.find(name);
		if (it != m_sprLookUp.end()){
			return 0;
		}

		SpriteHandle handle = m_sprites.size();
		m_sprites.emplace_back(spr);

		m_sprLookUp[name] = handle;
		m_sprLookUpInv[handle] = name;
		return handle;
	}

	/// This sprite wont be avaible through getSprite(name)
	SpriteHandle SpriteManager::createSprite(const Sprite& spr)
	{
		SpriteHandle handle = m_sprites.size();
		m_sprites.emplace_back(spr);

		return handle;
	}

	void SpriteManager::removeSprite(SpriteHandle hdl)
	{
		m_freeSlots.emplace_back(hdl);

		auto it = m_sprLookUpInv.find(hdl);
		if (it != m_sprLookUpInv.end()){
			m_sprLookUp.erase(it->second);
			m_sprLookUpInv.erase(it);
		}
	}

	SpriteHandle SpriteManager::getSpriteHandle(const std::string& name)
	{
		auto it = m_sprLookUp.find(name);
		if (it != m_sprLookUp.end())
			return it->second;

		return 0;
	}

	const Sprite* SpriteManager::getSprite(SpriteHandle hdl)
	{
#ifdef _DEBUG
		if (m_sprites.size() <= hdl){
			//LOGTO(Warning) << "SpriteManager: Invalid SpriteHandle!" << endlog;
			return nullptr;
		}
#endif

		return &m_sprites[hdl];
	}


}