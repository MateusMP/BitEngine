//#include "Platform/video/VideoRenderer.h"
//#include "Platform/video/Sprite2DRenderer.h"
//
//namespace BitEngine {
//
//Sprite2DRenderer::Sprite2DRenderer(EntitySystem* es, ResourceLoader* resourceLoader)
//    : ComponentProcessor(es), m_resourceLoader(resourceLoader)
//{
//    // DEFAULT_SPRITE
//    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_NONE);
//    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
//    m_sprite_materials[Sprite2DRenderer::DEFAULT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
//    // TRANSPARENT_SPRITE
//    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
//    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
//    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
//    m_sprite_materials[Sprite2DRenderer::TRANSPARENT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
//    // EFFECT_SPRITE
//    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
//    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
//    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
//    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setBlendEquation(BlendEquation::ADD);
//    m_sprite_materials[Sprite2DRenderer::EFFECT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
//}
//
//Sprite2DRenderer::~Sprite2DRenderer()
//{
//}
//
///*
//Code version that uses instanced rendering.
//The difference is that we dont need to define all vertices,
//Since the sprite is known to be a square, the shader already knows the vertices and just need a transform, texture and uv.
//void Sprite2DRenderer::prepare_instanced(Sprite2DBatch& batch)
//{
//    std::vector<Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;
//
//    m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
//    // Prepare for all instances
//    m_batch->prepare(batchInstances.size());
//
//    // Setup batch data
//    if (!batchInstances.empty())
//    {
//        Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(newRefs.m_textureContainer);
//        texture->diffuse = batch.bid.texture;
//
//        Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(newRefs.u_modelMatrixContainer);
//        Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(newRefs.m_ptnContainer);
//
//        const size_t instanceCount = batchInstances.size();
//        for (size_t i = 0; i < instanceCount; ++i)
//        {
//            const Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];
//
//            modelMatrices[i].modelMatrix = inst.transform.m_global;
//
//            vertices[i].textureUV = inst.sprite.sprite->getUV();
//        }
//    }
//}*/
//
//}