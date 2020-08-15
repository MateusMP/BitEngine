#pragma once

#include <BitEngine/Core/Assert.h>
#include <Platform/video/VideoRenderer.h>
#include <Platform/opengl/GL2/GL2Shader.h>
#include <Platform/opengl/GL2/GL2Driver.h>

#include <BitEngine/Core/Graphics/Sprite2D.h>

#include "Game/Common/MainMemory.h"

class Sprite2DBatch
{
public:
    struct SpriteBatchInstance {
        SpriteBatchInstance(const BitEngine::Mat3& t, const BitEngine::Sprite* s)
            : transform(t), sprite(s)
        {}

        SpriteBatchInstance& operator= (const SpriteBatchInstance& other) {
            return *(new(this)SpriteBatchInstance(other));
        }

        const BitEngine::Mat3& transform;
        const BitEngine::Sprite* sprite;
    };

    struct BatchIdentifier {
        BatchIdentifier(u32 _layer, const BitEngine::Material* mat, const BitEngine::Texture* tex)
            : layer(_layer), material(mat), texture(tex)
        {}

        bool operator<(const BatchIdentifier& o) const {
            return layer < o.layer || material < o.material || texture < o.texture;
        }

        u32 layer;
        const BitEngine::Material* material;
        const BitEngine::Texture* texture;
    };


    friend class Sprite2DRenderer;
public:
    Sprite2DBatch(const BatchIdentifier& bi)
        : bid(bi)
    {}

    void clear() {
        batchInstances.clear();
    }

    BatchIdentifier bid;
    std::vector<SpriteBatchInstance> batchInstances;
};

class GLSprite2DRenderer {

    // Instanced rendering
    struct Sprite2D_DD_new
    {
        struct PTNContainer {
            glm::vec2 position;
            glm::vec2 textureUV;
        };

        struct ModelMatrixContainer {
            glm::mat3 modelMatrix;
        };

        struct CamMatricesContainer {
            glm::mat4 view;
        };

        struct TextureContainer {
            const BitEngine::Texture* diffuse;
        };

        void init(BitEngine::Shader* shader)
        {
            using namespace BitEngine;
            m_ptnContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 0);
            u_modelMatrixContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Vertex, 1);

            u_viewMatrixContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 0);
            m_textureContainer = shader->getDefinition().getReferenceToContainer(DataUseMode::Uniform, 1);
        }

        BitEngine::ShaderDataReference m_ptnContainer;
        BitEngine::ShaderDataReference u_modelMatrixContainer;
        BitEngine::ShaderDataReference u_viewMatrixContainer;
        BitEngine::ShaderDataReference m_textureContainer;
    };

public:
    void init(BitEngine::ResourceLoader* loader) {
        const char* SPRITE_2D_SHADER_PATH = "sprite2Dshader";
        m_shader = loader->getResource<BitEngine::Shader>(SPRITE_2D_SHADER_PATH);
        if (!m_shader.isValid()) {
            LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Failed to load sprite 2D shader: " << SPRITE_2D_SHADER_PATH;
            return;
        }

        m_newRefs.init(m_shader.get());

        // DEFAULT_SPRITE
        using namespace BitEngine;
        m_sprite_materials[SpriteModes::DEFAULT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_NONE);
        m_sprite_materials[SpriteModes::DEFAULT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
        m_sprite_materials[SpriteModes::DEFAULT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
        // TRANSPARENT_SPRITE
        m_sprite_materials[SpriteModes::TRANSPARENT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
        m_sprite_materials[SpriteModes::TRANSPARENT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
        m_sprite_materials[SpriteModes::TRANSPARENT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
        m_sprite_materials[SpriteModes::TRANSPARENT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
        // EFFECT_SPRITE
        m_sprite_materials[SpriteModes::EFFECT_SPRITE].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
        m_sprite_materials[SpriteModes::EFFECT_SPRITE].setState(RenderConfig::TEXTURE_2D, true);
        m_sprite_materials[SpriteModes::EFFECT_SPRITE].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
        m_sprite_materials[SpriteModes::EFFECT_SPRITE].setBlendEquation(BlendEquation::ADD);
        m_sprite_materials[SpriteModes::EFFECT_SPRITE].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
    }

    BitEngine::BlendMaterial m_sprite_materials[3];

    void process(const Render2DBatchCommand& cmd) {
        BE_PROFILE_FUNCTION();

        if (!m_batch)
        {
            if (!m_shader->isReady()) {
                LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Skipping rendering until shader is loaded";
                return;
            }
            else {
                ((BitEngine::GL2Shader*)m_shader.get())->setupBatch(m_batch);
            }
        }

        m_batch->clear();


        if (cmd.count <= 0) {
            return;
        }

        u8 _matrixBuffer[sizeof(glm::mat3) * 256];
        BitEngine::MemoryArena matrixBuffer;
        matrixBuffer.init(_matrixBuffer, sizeof(_matrixBuffer));

        // Fix nulls
        for (u32 i = 0; i < cmd.count; ++i) {
            if (cmd.data[i].material == nullptr) {
                cmd.data[i].material = &m_sprite_materials[2];
            }
        }

        // Sort for batching
        {
            BE_PROFILE_SCOPE("Sort models");
            std::sort(cmd.data, cmd.data + cmd.count, [](const Sprite2D& a, const Sprite2D& b) {
                if (a.layer != b.layer) {
                    return a.layer < b.layer;
                }

                if (a.material != b.material) {
                    return a.material < b.material;
                }
                return (b.sprite->getTexture() < b.sprite->getTexture());
            });
        }

        for (auto& it : m_batches) {
            it.clear();
        }

        // Setup matrix array for every batch
        u32 batchIndices[32] = {};
        u32 currentIndex = 0;
        {
            BE_PROFILE_SCOPE("Preparing model matrices");
            const BitEngine::Material* lastMaterial = cmd.data[0].material;
            const BitEngine::Texture* lastTexture = cmd.data[0].sprite->getTexture().get();
            for (u32 i = 0; i < cmd.count; ++i) {
                const Sprite2D& spr = cmd.data[i];
                currentIndex += spr.sprite->getTexture().get() != lastTexture || spr.material != lastMaterial;
                BE_ASSERT(currentIndex < 32);
                batchIndices[currentIndex] = i;
                matrixBuffer.push<BitEngine::Mat3>(spr.transform);
                lastMaterial = spr.material;
                lastTexture = spr.sprite->getTexture().get();

                Sprite2DBatch::BatchIdentifier idtf(spr.layer, spr.material, spr.sprite->getTexture().get());
                const auto& it = m_batchesMap.find(idtf);
                if (it != m_batchesMap.end()) {
                    m_batches[it->second].batchInstances.emplace_back(spr.transform, spr.sprite);
                }
                else {
                    m_batches.emplace_back(idtf);
                    size_t id = m_batches.size() - 1;
                    m_batchesMap.emplace(idtf, id);
                    m_batches[id].bid = idtf;
                    m_batches[id].batchInstances.emplace_back(spr.transform, spr.sprite);
                }
            }
            currentIndex++;
        }

        u8 _vertexBuffer[sizeof(Shader3DSimple::Vertex) * 4 * 1024];
        BitEngine::MemoryArena vertexBuffer;

        for (auto& it : m_batches) {
            prepare_new(it, cmd.view);
            m_batch->load();
            BitEngine::GLVideoDriver::configure((BitEngine::BlendMaterial*)it.bid.material);
            m_batch->render(m_shader.get());
        }

    }
private:

    void prepare_new(const Sprite2DBatch& batch, const BitEngine::Mat4& viewProj)
    {
        BE_PROFILE_FUNCTION();
        const std::vector<Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;

        Sprite2D_DD_new::CamMatricesContainer* view = m_batch->getShaderDataAs<Sprite2D_DD_new::CamMatricesContainer>(m_newRefs.u_viewMatrixContainer);
        if (view) {
            view->view = viewProj;
        }

        m_batch->setVertexRenderMode(BitEngine::VertexRenderMode::TRIANGLES);
        // Prepare for all instances
        m_batch->prepare(batchInstances.size() * 6);

        // Setup batch data
        if (!batchInstances.empty())
        {
            Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(m_newRefs.m_textureContainer);
            texture->diffuse = batch.bid.texture;

            Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(m_newRefs.u_modelMatrixContainer);
            Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(m_newRefs.m_ptnContainer);

            const glm::vec2 vertex_pos[4] = {
                    glm::vec2(0.0f, 0.0f),
                    glm::vec2(1.0f, 0.0f),
                    glm::vec2(0.0f, 1.0f),
                    glm::vec2(1.0f, 1.0f)
            };
            const size_t instanceCount = batchInstances.size();
            for (size_t i = 0; i < instanceCount; ++i)
            {
                const Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];

                modelMatrices[i].modelMatrix = inst.transform;

                const glm::vec2 sizes(inst.sprite->getWidth(), inst.sprite->getHeight());
                const glm::vec2 offsets(-inst.sprite->getOffsetX(), -inst.sprite->getOffsetY());
                const glm::vec2 off_siz = offsets * sizes;

                const glm::vec4& uvrect = inst.sprite->getUV();

                vertices[i * 6 + 0].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[0] * sizes + off_siz, 1)); vertices[i * 6 + 0].textureUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw glm::vec2(0, 1);
                vertices[i * 6 + 1].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[1] * sizes + off_siz, 1)); vertices[i * 6 + 1].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
                vertices[i * 6 + 2].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[2] * sizes + off_siz, 1)); vertices[i * 6 + 2].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(1, 0);
                vertices[i * 6 + 3].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[2] * sizes + off_siz, 1)); vertices[i * 6 + 3].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(0, 1);
                vertices[i * 6 + 4].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[1] * sizes + off_siz, 1)); vertices[i * 6 + 4].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
                vertices[i * 6 + 5].position = glm::vec2(inst.transform * glm::vec3(vertex_pos[3] * sizes + off_siz, 1)); vertices[i * 6 + 5].textureUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy glm::vec2(1, 1);              
                             
            }
        }
    }

    BitEngine::RR<BitEngine::Shader> m_shader;
    BitEngine::Lazy<BitEngine::GL2Batch> m_batch;
    std::vector<Sprite2DBatch> m_batches;
    std::map<Sprite2DBatch::BatchIdentifier, size_t> m_batchesMap;
    Sprite2D_DD_new m_newRefs;
};