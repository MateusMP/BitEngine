#include "BitEngine/Game/ECS/Transform2DComponent.h"

#include <math.h>

namespace BitEngine {

Transform2DComponent::Transform2DComponent()
    : position(0.0f, 0.0f)
    , scale(1.0f, 1.0f)
    , rotation(0.0f)
    , m_dirty(true)
{
}

Transform2DComponent::~Transform2DComponent()
{
}

const Vec2& Transform2DComponent::getLocalPosition() const
{
    return position;
}

void Transform2DComponent::setLocalPosition(const Vec2& p)
{
    position = p;
    m_dirty = true;
}

const Vec2& Transform2DComponent::getLocalScale() const
{
    return scale;
}

void Transform2DComponent::setLocalScale(const Vec2& s)
{
    scale = s;
    m_dirty = true;
}

float Transform2DComponent::getLocalRotation() const
{
    return rotation;
}

void Transform2DComponent::setLocalRotation(float rad)
{
    rotation = rad;
    m_dirty = true;
}
}
