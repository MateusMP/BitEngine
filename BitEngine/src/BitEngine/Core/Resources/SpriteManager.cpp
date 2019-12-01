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
        new (sprite) Sprite(meta);
    }
    return sprite;
}

void SpriteManager::readJsonProperties(DevResourceLoader* devloader, nlohmann::json& props, ResourceManager* manager, BaseResource* resource)
{
    /*
    Sprite* sprite = static_cast<Sprite*>(resource);
    const std::string& textureRef = props["texture"].get<std::string>();

    u16 w = props["width"].get<u16>();
    u16 h = props["height"].get<u16>();
    float ox = props["ox"].get<float>();
    float oy = props["oy"].get<float>();

    auto& uvContainer = props["uv"];
    glm::vec4 uv(uvContainer["xi"].get<float>(), uvContainer["yi"].get<float>(),
        uvContainer["xf"].get<float>(), uvContainer["yf"].get<float>());
    bool transparent = props["transparent"].get<int>() > 0;

    ResourceMeta* textureMeta = devloader->findMeta(textureRef);
    RR<Texture> texture = devloader->getResource<Texture>(textureMeta);
    if (!texture.isValid()) {
        LOG(EngineLog, BE_LOG_ERROR) << "Coulnd't find texture resource " << textureMeta << " for sprite " << resource->getResourceId();
    }

    sprite->~Sprite(); // Make sure we release references before creating new ones

    new (sprite) Sprite(sprite->getMeta(), texture, w, h, ox, oy, uv, transparent);
    */
}

void SpriteManager::reloadResource(BaseResource* resource)
{
    // Do nothing
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