#pragma once

#include <glm/common.hpp>

#include "bitengine/Core/Graphics/VideoRenderer.h"
#include "bitengine/Core/ECS/EntitySystem.h"
#include "bitengine/Core/Resources/SpriteManager.h"

#include "bitengine/Core/Graphics/Shader.h"
#include "bitengine/Core/Graphics/Material.h"
#include "bitengine/Core/ECS/Transform2DProcessor.h"
#include "bitengine/Core/ECS/Camera2DComponent.h"

namespace BitEngine
{

class Sprite2DComponent : public Component<Sprite2DComponent>
{
public:
    const static Material* DEFAULT_SPRITE;
    const static Material* TRANSPARENT_SPRITE;
    const static Material* EFFECT_SPRITE;

    Sprite2DComponent(u32 _layer, RR<Sprite> spr, const Material* mat)
        : layer(_layer), alpha(1.0f), sprite(spr), material(mat)
    {
    }

    s32 layer;
    float alpha;
    RR<Sprite> sprite;
    const Material* material;
};

template<>
class BE_API ComponentHolder<Sprite2DComponent> : public BaseComponentHolder
{
public:
    ComponentHolder()
        : BaseComponentHolder(sizeof(Sprite2DComponent))
    {

    }

    Messenger<MsgComponentCreated<Sprite2DComponent>> componentCreatedSignal;
    Messenger<MsgComponentDestroyed<Sprite2DComponent>> componentDestroyedSignal;

    bool init() override {
        return true;
    }

    void sendDestroyMessage(EntityHandle entity, ComponentHandle component) override {
        componentDestroyedSignal.emit(MsgComponentDestroyed<Sprite2DComponent>(entity, Sprite2DComponent::getComponentType(), component));
    }

    template<typename ... Args>
    void initializeComponent(Sprite2DComponent* outPtr, Args ...args) {
        initComponent(outPtr, args...);
    }


private:
    template<typename ... Args>
    void initComponent(Sprite2DComponent* outPtr, Args ...args) {
        new (outPtr) Sprite2DComponent(args...);
    }

    void initComponent(Sprite2DComponent* outPtr) {
        new (outPtr) Sprite2DComponent(0, RR<Sprite>::invalid(), Sprite2DComponent::DEFAULT_SPRITE);
    }
};


}
