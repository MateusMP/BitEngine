//#pragma once
//
//#include "BitEngine/Core/Profiler.h"
//#include "BitEngine/Core/Graphics/Sprite2D.h"
//#include "Platform/video/BlendMaterial.h"
//
//
//
//namespace BitEngine {
//
//
//class BE_API Sprite2DRenderer : public ComponentProcessor
//{
//public:
//    Sprite2DRenderer(EntitySystem* es, ResourceLoader* resourceLoader);
//    ~Sprite2DRenderer();
//
//
//    constexpr static u32 DEFAULT_SPRITE = 0;
//    constexpr static u32 TRANSPARENT_SPRITE = 1;
//    constexpr static u32 EFFECT_SPRITE = 2;
//
//    const Material* getMaterial(u32 type) {
//        return &m_sprite_materials[type];
//    }
//
//    void processEntities(ComponentRef<Camera2DComponent> camera, RenderQueue* queue) {
//        BE_PROFILE_FUNCTION(BitEngine::EngineLog);
//
//        Render2DBatchCommand* batch = queue->initRenderBatch2D();
//
//        const glm::vec4 viewScreen = camera->getWorldViewArea();
//        batch->view = camera->getMatrix();
//
//        // Build batch
//        // TODO: Make loop append entries to render queue
//        // Individual entries should be prepared for rendering
//        // by the rendering implementation
//        getES()->forEach<SceneTransform2DComponent, Sprite2DComponent>(
//            [=](ComponentRef<SceneTransform2DComponent>&& transform, ComponentRef<Sprite2DComponent>&& sprite)
//        {
//            if (insideScreen(viewScreen, transform->getGlobal(), 64))
//            {
//                if (sprite->material == nullptr) {
//                    sprite->material = &m_sprite_materials[0];
//                }
//
//                queue->pushSprite2D(batch, sprite, transform->getGlobal());
//            }
//        });
//    }
//
//private:
//    
//    static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, float radius)
//    {
//        const float kX = matrix[2][0] + radius;
//        const float kX_r = matrix[2][0] - radius;
//        const float kY = matrix[2][1] + radius;
//        const float kY_b = matrix[2][1] - radius;
//
//        if (kX < screen.x) {
//            // printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
//            return false;
//        }
//        if (kX_r > screen.z) {
//            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
//            return false;
//        }
//        if (kY < screen.y) {
//            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
//            return false;
//        }
//        if (kY_b > screen.w) {
//            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
//            return false;
//        }
//
//        return true;
//    }
//    
//    ResourceLoader* m_resourceLoader;
//    BlendMaterial m_sprite_materials[3];
//};
//
//}