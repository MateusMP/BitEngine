#pragma once

#include <algorithm>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Math.h"
#include "BitEngine/Core/Graphics/Texture.h"
#include "BitEngine/Core/Resources/PropertyHolder.h"

namespace BitEngine {

typedef unsigned short SpriteHandle;

class Sprite final : public BaseResource
{
public:

    Sprite()
        : BaseResource(nullptr),
        m_width(16), m_height(16),
        m_offsetX(0), m_offsetY(0), m_uvrect(0.0f, 0.0f, 1.0f, 1.0f), m_transparent(false)
    {
        calculateMaxRadius();
    }

    Sprite(ResourceMeta* meta) : BaseResource(meta) {

    }

    Sprite(ResourceMeta* meta, const RR<Texture>& texture, int w, int h, float oX, float oY, const glm::vec4& _uvrect, bool _transparent = false)
        : BaseResource(meta), m_textureID(texture), m_width(w), m_height(h), m_offsetX(oX), m_offsetY(oY), m_uvrect(_uvrect), m_transparent(_transparent)
    {
        calculateMaxRadius();
    }

    void release() {
        m_textureID.invalidate();
    }

    float getMaxRadius() const {
        return m_maxRadius;
    }

    const RR<Texture>& getTexture() const {
        return m_textureID;
    }

    int getWidth() const {
        return m_width;
    }

    int getHeight() const {
        return m_height;
    }

    float getOffsetX() const {
        return m_offsetX;
    }

    float getOffsetY() const {
        return m_offsetY;
    }

    bool isTransparent() const {
        return m_transparent;
    }

    const glm::vec4& getUV() const {
        return m_uvrect;
    }

    const Sprite& operator =(const Sprite& spr);

    template<typename Serializer>
    static void serialize(Serializer* s, const BaseResource* obj) {
        const Sprite* sprite = static_cast<const Sprite*>(obj);
        s->write("texture", sprite->m_textureID);
        s->write("width", sprite->m_width);
        s->write("height", sprite->m_height);
        s->write("offsetX", sprite->m_offsetX);
        s->write("offsetY", sprite->m_offsetY);
        s->write("uvrect", sprite->m_uvrect);
        s->write("transparent", (u8)sprite->m_transparent);
    }

    static void read(PropertyHolder* s, BaseResource* obj) {
        Sprite* sprite = static_cast<Sprite*>(obj);
        s->read("texture", &sprite->m_textureID);
        s->read("width", &sprite->m_width);
        s->read("height", &sprite->m_height);
        s->read("offsetX", &sprite->m_offsetX);
        s->read("offsetY", &sprite->m_offsetY);
        s->read("uvrect", &sprite->m_uvrect);
        s->read("transparent", (u8*)&sprite->m_transparent);
    }

private:
    void calculateMaxRadius();

    RR<Texture> m_textureID;
    s32 m_width;
    s32 m_height;
    float m_offsetX;
    float m_offsetY;

    /**
        xw--zw (1,1)
        |    |
        xy--zy
    (0,0)
    */
    glm::vec4 m_uvrect;
    bool m_transparent;

    float m_maxRadius;
};

using RSprite = RR<Sprite>;

// INL

inline const Sprite& Sprite::operator =(const Sprite& spr) {
    m_textureID = spr.m_textureID;
    m_width = spr.m_width;
    m_height = spr.m_height;
    m_offsetX = spr.m_offsetX;
    m_offsetY = spr.m_offsetY;
    m_uvrect = spr.m_uvrect;
    m_transparent = spr.m_transparent;

    calculateMaxRadius();
    return *this;
}

inline void Sprite::calculateMaxRadius()
{
    float maxW = static_cast<float>(m_width);
    if (m_offsetX <= 0) {
        maxW *= 1 - m_offsetX;
    }
    else {
        maxW *= std::max(1.0f - m_offsetX, m_offsetX);
    }

    float maxH = static_cast<float>(m_height);
    if (m_offsetY <= 0) {
        maxH *= 1 - m_offsetY;
    }
    else {
        maxH *= std::max(1.0f - m_offsetY, m_offsetY);
    }

    m_maxRadius = std::sqrt(maxW*maxW + maxH * maxH);
}

}
