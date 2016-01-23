#include "Core/SpriteManager.h"

#include "Core/Logger.h"
#include "Core/TextureManager.h"

namespace BitEngine{

	SpriteManager::SpriteManager(TextureManager* textureManager)
		: m_textureManager(textureManager)
	{
	}

	bool SpriteManager::Init()
	{
		m_sprites.clear();
		m_freeSlots.clear();
		m_sprLookUp.clear();
		m_sprLookUpInv.clear();

		m_sprites.emplace_back(m_textureManager->getErrorTexture()->getID(), 64, 64, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1), false); // First sprite is INVALID

		return true;
	}

	SpriteHandle SpriteManager::createSprite(const std::string& name, const Sprite& spr)
	{
		auto it = m_sprLookUp.find(name);
		if (it != m_sprLookUp.end()){
			return 0;
		}

		SpriteHandle handle = (SpriteHandle)m_sprites.size();
		m_sprites.emplace_back(spr);

		m_sprLookUp[name] = handle;
		m_sprLookUpInv[handle] = name;
		return handle;
	}

	/// This sprite wont be avaible through getSprite(name)
	SpriteHandle SpriteManager::createSprite(const Sprite& spr)
	{
		SpriteHandle handle = (SpriteHandle)m_sprites.size();
		m_sprites.emplace_back(spr);

		return handle;
	}

	void SpriteManager::removeSprite(SpriteHandle hdl)
	{
		if (hdl == 0)
			return;

		m_freeSlots.emplace_back(hdl);

		auto it = m_sprLookUpInv.find(hdl);
		if (it != m_sprLookUpInv.end()){
			m_sprLookUp.erase(it->second);
			m_sprLookUpInv.erase(it);
		}
	}

	bool SpriteManager::replaceSprite(SpriteHandle hdl, const Sprite& spr)
	{
		if (hdl == 0)
			return false;

		if (m_sprites.size() <= hdl){
			LOG(EngineLog, BE_LOG_WARNING) << "SpriteManager: replaceSprite Invalid SpriteHandle: " <<  hdl;
			return false;
		}

		m_sprites[hdl] = spr;
		return true;
	}

	SpriteHandle SpriteManager::getSpriteHandle(const std::string& name) const
	{
		auto it = m_sprLookUp.find(name);
		if (it != m_sprLookUp.end())
			return it->second;

		return 0;
	}

	const Sprite* SpriteManager::getSprite(SpriteHandle hdl) const
	{
#ifdef _DEBUG
		if (m_sprites.size() <= hdl){
			LOG(EngineLog, BE_LOG_WARNING) << "SpriteManager: Invalid SpriteHandle!";
			return nullptr;
		}
#endif

		return &m_sprites[hdl];
	}


}