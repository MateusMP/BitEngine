#include "BitEngine/Core/Resources/SpriteManager.h"

#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/Graphics/Texture.h"
#include "BitEngine/Core/Resources/DevResourceLoader.h"

namespace BitEngine {

SpriteManager::SpriteManager()
{
}

ResourceMeta nullSprite;

bool SpriteManager::init()
{
    dynamicSprites.reserve(1000);
    ResourceMeta* meta = &nullSprite;
    u16 defId = sprites.addResource(meta);
    Sprite* nullSprite = sprites.getResourceAddress(defId);
    new (nullSprite) Sprite(meta, {}, 64, 64, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1), false);

    return true;
}

Sprite* SpriteManager::createSprite(const std::string& name, const Sprite& spr)
{
    dynamicSprites.emplace_back(ResourceMeta());
    ResourceMeta* meta = &dynamicSprites[dynamicSprites.size()];
    if (meta != nullptr) {
        u16 id = sprites.addResource(meta);
        Sprite* sprite = sprites.getResourceAddress(id);
        new (sprite) Sprite(spr);
        return sprite;
    }
    else {
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

BaseResource* SpriteManager::loadResource(ResourceMeta* meta, PropertyHolder* props)
{
    Sprite* sprite = sprites.findResource(meta);
    if (sprite == nullptr) {
        u16 id = sprites.addResource(meta);
        sprite = sprites.getResourceAddress(id);
        new (sprite) Sprite(meta);

        props->readObject("sprite", sprite);
    }
    return sprite;
}

void SpriteManager::reloadResource(BaseResource* resource)
{
    // Do nothing
}

ptrsize SpriteManager::getCurrentRamUsage() const
{
    return sizeof(sprites);
}

u32 SpriteManager::getCurrentGPUMemoryUsage() const
{
    return u32(0);
}
}