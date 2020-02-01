#include "BitEngine/Core/Graphics/Sprite2DRenderer.h"

namespace BitEngine {

Sprite2DRenderer::Sprite2DRenderer(EntitySystem* es, ResourceLoader* resourceLoader)
    : ComponentProcessor(es), m_resourceLoader(resourceLoader)
{
    // DEFAULT_SPRITE
    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_NONE);
    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    // TRANSPARENT_SPRITE
    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    // EFFECT_SPRITE
    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setBlendEquation(BlendEquation::ADD);
    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
}

Sprite2DRenderer::~Sprite2DRenderer()
{
}

void Sprite2DRenderer::setActiveCamera(ComponentRef<Camera2DComponent>& handle)
{
    m_activeCamera = handle;
}

const std::vector<Sprite2DBatch>& Sprite2DRenderer::GenerateRenderData()
{
    buildBatchInstances();
    return m_batches;
}

void Sprite2DRenderer::buildBatchInstances()
{
    // LOG_FUNCTION_TIME(BitEngine::EngineLog);
    if (!m_activeCamera) { return; }

    for (Sprite2DBatch& it : m_batches) {
        it.batchInstances.clear();
    }

    const glm::vec4 viewScreen = m_activeCamera->getWorldViewArea();

    // Build batch
    // TODO: Make loop append entries to render queue
    // Individual entries should be prepared for rendering
    // by the rendering implementation
    getES()->forEach<SceneTransform2DComponent, Sprite2DComponent>(
        [=](ComponentRef<SceneTransform2DComponent>&& transform, ComponentRef<Sprite2DComponent>&& sprite)
    {
        if (insideScreen(viewScreen, transform->getGlobal(), 64))
        {
            if (sprite->material == nullptr) {
                sprite->material = &m_sprite_materials[0];
            }

            const Texture* texture = sprite->sprite->getTexture().get();
            Sprite2DBatch::BatchIdentifier idtf(sprite->layer, sprite->material, sprite->sprite->getTexture().get());
            const auto& it = m_batchesMap.find(idtf);
            if (it != m_batchesMap.end()) {
                m_batches[it->second].batchInstances.emplace_back(transform.ref(), sprite.ref());
            } else {
                m_batches.emplace_back(idtf);
                size_t id = m_batches.size() - 1;
                m_batchesMap.emplace(idtf, id);
                m_batches[id].bid = idtf;
                m_batches[id].batchInstances.emplace_back(transform.ref(), sprite.ref());
            }
        }
    });
}


/*
Code version that uses instanced rendering.
The difference is that we dont need to define all vertices,
Since the sprite is known to be a square, the shader already knows the vertices and just need a transform, texture and uv.
void Sprite2DRenderer::prepare_instanced(Sprite2DBatch& batch)
{
    std::vector<Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;

    m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
    // Prepare for all instances
    m_batch->prepare(batchInstances.size());

    // Setup batch data
    if (!batchInstances.empty())
    {
        Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(newRefs.m_textureContainer);
        texture->diffuse = batch.bid.texture;

        Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(newRefs.u_modelMatrixContainer);
        Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(newRefs.m_ptnContainer);

        const size_t instanceCount = batchInstances.size();
        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];

            modelMatrices[i].modelMatrix = inst.transform.m_global;

            vertices[i].textureUV = inst.sprite.sprite->getUV();
        }
    }
}*/

}