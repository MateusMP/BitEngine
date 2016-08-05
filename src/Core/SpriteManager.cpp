#include "Core/SpriteManager.h"

#include "Core/Logger.h"
#include "Core/Resources/ResourceManager.h"
#include "Core/Graphics/ITexture.h"

namespace BitEngine{

	SpriteManager::SpriteManager()
	{
	}

	bool SpriteManager::init()
	{
		Sprite* nullSprite = sprites.getResourceAddress(0);
		new (nullSprite) Sprite(resourceLoader->getResource<ITexture>("default/texture"), 64, 64, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1), false);

		return true;
	}

	Sprite* SpriteManager::createSprite(const std::string& name, const Sprite& spr)
	{
		ResourceMeta* meta = resourceLoader->includeMeta("sprites", name, "SPRITE");
		if (meta != nullptr)
		{
			u16 id = sprites.addResource(meta);
			Sprite* sprite = sprites.getResourceAddress(id);
			new(sprite)Sprite(spr);
			return sprite;
		}
		else
		{
			return nullptr;
		}
	}

	void SpriteManager::update()
	{
	}
	
	BaseResource* SpriteManager::loadResource(ResourceMeta* meta)
	{
		Sprite* sprite = sprites.findResource(meta);
		if (sprite == nullptr)
		{
			u16 id = sprites.addResource(meta);
			sprite = sprites.getResourceAddress(id);

			loadSpriteData(meta, sprite);
			
		}
		return sprite;
	}

	void SpriteManager::loadSpriteData(ResourceMeta* meta, Sprite* sprite)
	{
		ResourcePropertyContainer& props = meta->properties;

		const std::string& textureMeta = props["texture"].getValueString();
		u16 w = props["w"].getValueInt();
		u16 h = props["h"].getValueInt();
		double ox = props["ox"].getValueDouble();
		double oy = props["oy"].getValueDouble();

		ResourcePropertyContainer& uvContainer = props["uv"];
		glm::vec4 uv(uvContainer["xi"].getValueDouble(), uvContainer["yi"].getValueDouble(), 
					 uvContainer["xf"].getValueDouble(), uvContainer["yf"].getValueDouble());
		bool transparent = props["transparent"].getValueInt() > 0;

		ITexture* texture = resourceLoader->getResource<ITexture>(textureMeta);
		if (texture == nullptr) {
			LOG(EngineLog, BE_LOG_ERROR) << "Coulnd't find texture resource " << textureMeta << " for sprite " << meta->resourceName;
		}

		new (sprite) Sprite(texture, w, h, ox, oy, uv, transparent);
	}

	u32 SpriteManager::getCurrentRamUsage() const
	{
		return sizeof(sprites);
	}

	u32 SpriteManager::getCurrentGPUMemoryUsage() const
	{
		return u32(0);
	}


}