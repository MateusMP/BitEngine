#pragma once

#include "BitEngine/Core/Resources/ResourceManager.h"

namespace BitEngine
{
class Texture : public BaseResource
{
public:
    Texture(ResourceMeta* meta) : BaseResource(meta) {}
    virtual ~Texture() {}
    virtual u32 getTextureID() const = 0;
    virtual u32 getTextureType() const = 0;

    template<typename Serializer>
    static void serialize(Serializer* s, const BaseResource* obj) {

    }

    template<typename Deserializer>
    static void deserialize(Deserializer* s, BaseResource* obj) {

    }
};
using RTexture = RR<Texture>;
}
