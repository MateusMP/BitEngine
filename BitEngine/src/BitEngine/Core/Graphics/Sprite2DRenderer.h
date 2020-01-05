#pragma once

#include "BitEngine/Core/Graphics/Sprite2D.h"

namespace BitEngine {

class Sprite2DBatch
{
public:
    struct SpriteBatchInstance {
        SpriteBatchInstance(const SceneTransform2DComponent& t, const Sprite2DComponent& s)
            : transform(t), sprite(s)
        {}

        SpriteBatchInstance& operator= (const SpriteBatchInstance& other) {
            return *(new(this)SpriteBatchInstance(other));
        }

        const SceneTransform2DComponent& transform;
        const Sprite2DComponent& sprite;
    };

    struct BatchIdentifier {
        BatchIdentifier(u32 _layer, const Material* mat, const Texture* tex)
            : layer(_layer), material(mat), texture(tex)
        {}

        bool operator<(const BatchIdentifier& o) const {
            return layer < o.layer || material < o.material || texture < o.texture;
        }

        u32 layer;
        const Material* material;
        const Texture* texture;
    };


    friend class Sprite2DRenderer;
public:
    Sprite2DBatch(const BatchIdentifier& bi)
        : bid(bi)
    {}

    BatchIdentifier bid;
    std::vector<SpriteBatchInstance> batchInstances;
};


class BE_API Sprite2DRenderer : public ComponentProcessor
{
public:
    Sprite2DRenderer(EntitySystem* es, ResourceLoader* resourceLoader);
    ~Sprite2DRenderer();

    void setActiveCamera(ComponentRef<Camera2DComponent>& handle);
    const std::vector<Sprite2DBatch>& GenerateRenderData();

    constexpr static u32 DEFAULT_SPRITE = 0;
    constexpr static u32 TRANSPARENT_SPRITE = 1;
    constexpr static u32 EFFECT_SPRITE = 2;

    const Material* getMaterial(u32 type) {
        return &m_sprite_materials[type];
    }

private:
    
    static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, float radius)
    {
        const float kX = matrix[2][0] + radius;
        const float kX_r = matrix[2][0] - radius;
        const float kY = matrix[2][1] + radius;
        const float kY_b = matrix[2][1] - radius;

        if (kX < screen.x) {
            // printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
            return false;
        }
        if (kX_r > screen.z) {
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
            return false;
        }
        if (kY < screen.y) {
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
            return false;
        }
        if (kY_b > screen.w) {
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
            return false;
        }

        return true;
    }

    void prepare_new(Sprite2DBatch& batch);

    void buildBatchInstances();

    std::map<Sprite2DBatch::BatchIdentifier, u32> m_batchesMap;
    std::vector<Sprite2DBatch> m_batches;
    ComponentRef<Camera2DComponent> m_activeCamera;
    ResourceLoader* m_resourceLoader;
    Material m_sprite_materials[3];
};

}