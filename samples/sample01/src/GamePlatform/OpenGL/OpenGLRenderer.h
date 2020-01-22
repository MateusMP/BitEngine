#pragma once

#include <Platform/opengl/GL2/GL2Driver.h>

#include <BitEngine/Core/Graphics/Sprite2DRenderer.h>

#include "Shader3DProcessor.h"
#include "Shader3DSimple.h"


// Sprite 2D batch

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
    }

    void process(const RenderSpriteBatch2DCommand& command) {
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

        for (auto& it : command.batch2d) {
            prepare_new(it, command.viewProj);
            m_batch->load();
            BitEngine::GLVideoDriver::configure(it.bid.material);
            m_batch->render(m_shader.get());
        }
    }
private:

    void prepare_new(const BitEngine::Sprite2DBatch& batch, const BitEngine::Mat4& viewProj)
    {
        BE_PROFILE_FUNCTION();
        const std::vector<BitEngine::Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;

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
                const BitEngine::Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];

                modelMatrices[i].modelMatrix = inst.transform.m_global;

                const glm::vec2 sizes(inst.sprite.sprite->getWidth(), inst.sprite.sprite->getHeight());
                const glm::vec2 offsets(-inst.sprite.sprite->getOffsetX(), -inst.sprite.sprite->getOffsetY());
                const glm::vec2 off_siz = offsets * sizes;

                vertices[i * 6 + 0].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[0] * sizes + off_siz, 1));
                vertices[i * 6 + 1].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
                vertices[i * 6 + 2].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
                vertices[i * 6 + 3].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
                vertices[i * 6 + 4].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
                vertices[i * 6 + 5].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[3] * sizes + off_siz, 1));

                const glm::vec4& uvrect = inst.sprite.sprite->getUV();
                vertices[i * 6 + 0].textureUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw glm::vec2(0, 1);
                vertices[i * 6 + 1].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
                vertices[i * 6 + 2].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(1, 0);
                vertices[i * 6 + 3].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(0, 1);
                vertices[i * 6 + 4].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
                vertices[i * 6 + 5].textureUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy glm::vec2(1, 1);
            }
        }
    }

    BitEngine::Lazy<BitEngine::GL2Batch> m_batch;
    BitEngine::RR<BitEngine::Shader> m_shader;
    Sprite2D_DD_new m_newRefs;
};

class GLModelRenderer {
public:
    void init(BitEngine::ResourceLoader* loader) {
        if (!m_shader.Init()) {
            LOG(GameLog(), BE_LOG_ERROR) << "Could not create Shader3DSimple";
        }

        m_meshRenderer = m_shader.CreateRenderer();
        if (m_meshRenderer == nullptr) {
            LOG(GameLog(), BE_LOG_ERROR) << "Could not create Shader3DSimple renderer";
            return;
        }
    }

    void process(const Render3DBatchCommand& cmd) {

        // TODO: parallelize this loop?
        for (u32 i = 0; i < cmd.count; ++i) {
            Model3D& model = cmd.data[i];
            m_meshRenderer->addMesh((Shader3DSimple::Mesh*)model.mesh, model.material, &model.transform);
        }

        m_meshRenderer->End();

        // Set up shader
        m_shader.LoadProjectionMatrix(cmd.projection);
        m_shader.LoadViewMatrix(cmd.view);
        m_shader.Bind();

        // Set up GL states
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Render
        m_meshRenderer->Render();
    }

private:
    Shader3DSimple m_shader;
    Shader3DSimple::BatchRenderer* m_meshRenderer;
};

class GLRenderer {
public:

    void init(BitEngine::ResourceLoader* loader) {
        spriteRenderer.init(loader);
        modelsRenderer.init(loader);
    }

    void render(RenderQueue* queue) {

        u32 count = queue->getCommandsCount();
        RenderCommand* commands = queue->getCommands();

        for (u32 i = 0; i < count; ++i) {
            switch (commands[i].command) {
            case Command::SCENE_BEGIN:
            {
                BE_PROFILE_SCOPE("Render SCENE_BEGIN");
                SceneBeginCommand& cmd = commands[i].data.sceneBegin;
                // driver->clearBufferColor(nullptr, BitEngine::ColorRGBA(0.7f, 0.2f, 0.3f, 0.f));
                BitEngine::GLVideoDriver::clearBufferColor(nullptr, BitEngine::ColorRGBA(0.3f, 0.7f, 0.3f, 0.f));
                BitEngine::GLVideoDriver::clearBuffer(nullptr, BitEngine::BufferClearBitMask::COLOR_DEPTH);
            }
            break;
            case Command::SPRITE_BATCH_2D:
            {
                BE_PROFILE_SCOPE("Render SPRITE_BATCH_2D");
                spriteRenderer.process(commands[i].data.renderSpriteBatch2D);
            }
            break;
            case SCENE_2D:
                break;
            case Command::SCENE_3D_BATCH:
                Render3DBatchCommand& cmd = commands[i].data.batch3d;
                modelsRenderer.process(cmd);
                break;
            }

        }

    }


    GLSprite2DRenderer spriteRenderer;
    GLModelRenderer modelsRenderer;
};

