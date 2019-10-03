#include "bitengine/Core/Resources/SpriteManager.h"

#include "bitengine/Core/Logger.h"
#include "bitengine/Core/Resources/ResourceManager.h"
#include "bitengine/Core/Graphics/Texture.h"

namespace BitEngine{

	SpriteManager::SpriteManager()
	{
	}

	bool SpriteManager::init()
	{
		ResourceMeta* meta = resourceLoader->includeMeta("default", "sprite", "SPRITE");
		u16 defId = sprites.addResource(meta);
		Sprite* nullSprite = sprites.getResourceAddress(defId);
		new (nullSprite) Sprite(meta, resourceLoader->getResource<Texture>("data/default/texture"), 64, 64, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1), false);

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

    void SpriteManager::shutdown()
    {
        for (Sprite& sprite : sprites.getResources()) {
            sprite.release();
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
		float ox = static_cast<float>(props["ox"].getValueDouble());
		float oy = static_cast<float>(props["oy"].getValueDouble());

		const ResourcePropertyContainer& uvContainer = props["uv"];
		glm::vec4 uv(uvContainer["xi"].getValueDouble(), uvContainer["yi"].getValueDouble(), 
					 uvContainer["xf"].getValueDouble(), uvContainer["yf"].getValueDouble());
		bool transparent = props["transparent"].getValueInt() > 0;

		RR<Texture> texture = resourceLoader->getResource<Texture>(textureMeta);
		if (!texture.isValid()) {
			LOG(EngineLog, BE_LOG_ERROR) << "Coulnd't find texture resource " << textureMeta << " for sprite " << meta->resourceName;
		}

		new (sprite) Sprite(meta, texture, w, h, ox, oy, uv, transparent);
	}

	void SpriteManager::reloadResource(BaseResource* resource)
	{
		Sprite* spr = static_cast<Sprite*>(resource);
		spr->~Sprite();
		loadSpriteData(resource->getMeta(), spr);
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