#pragma once

#include <BitEngine/Core/Assert.h>
#include <Platform/opengl/GL2/GL2Driver.h>

#include <BitEngine/Core/Graphics/Sprite2DRenderer.h>

#include "Shader3DSimple.h"


//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

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
        if (m_shader.Init() != BE_NO_ERROR) {
            LOG(GameLog(), BE_LOG_ERROR) << "Could not create Shader3DSimple";
            BE_INVALID_PATH("Shader3D Error");
        }
    }

    void destroy() {
        for (std::pair<BitEngine::Mesh*, Shader3DSimple::ShaderMesh> element : m_shaderMesh) {
            element.second.destroy();
        }
    }

    void process(const Render3DBatchCommand& cmd) {
        BE_PROFILE_FUNCTION();
        if (cmd.count <= 0) {
            return;
        }

        u8 _matrixBuffer[sizeof(glm::mat4) * 256];
        BitEngine::MemoryArena matrixBuffer;
        matrixBuffer.init(_matrixBuffer, sizeof(_matrixBuffer));
        
        // Sort for batching
        {
            BE_PROFILE_SCOPE("Sort models");
            std::sort(cmd.data, cmd.data + cmd.count, [](const Model3D& a, const Model3D& b) {
                if (a.material != b.material) {
                    return a.material < b.material;
                }
                return (b.mesh < b.mesh);
            });
        }

        // Setup matrix array for every batch
        u32 batchIndices[32] = {};
        u32 currentIndex = 0;
        {
            BE_PROFILE_SCOPE("Preparing model matrices");
            BitEngine::Material* lastMaterial = cmd.data[0].material;
            BitEngine::Mesh* lastMesh = cmd.data[0].mesh;
            for (u32 i = 0; i < cmd.count; ++i) {
                const Model3D& model = cmd.data[i];
                currentIndex += model.mesh != lastMesh || model.material != lastMaterial;
                BE_ASSERT(currentIndex < 32);
                batchIndices[currentIndex] = i;
                matrixBuffer.push<BitEngine::Mat4>(model.transform);
                lastMaterial = model.material;
                lastMesh = model.mesh;
            }
            currentIndex++;
        }

        // For now, ensure every mesh is already setup for rendering
        // Probably do this earlier during loading
        // TODO: Move this part to somewhere else
        u8 _vertexBuffer[sizeof(Shader3DSimple::Vertex) * 4 * 1024];
        BitEngine::MemoryArena vertexBuffer;

        for (int i = 0; i < currentIndex; ++i) {
            u32 end = batchIndices[i];

            BitEngine::Mesh* mesh = cmd.data[end].mesh;
            if (m_shaderMesh.find(mesh) == m_shaderMesh.end()) {
                vertexBuffer.init(_vertexBuffer, sizeof(_vertexBuffer));

                BitEngine::Mesh::DataArray indices = mesh->getIndicesData(0);
                BitEngine::Mesh::DataArray verts = mesh->getVertexData(BitEngine::Mesh::VertexDataType::Vertices);
                BitEngine::Mesh::DataArray texs = mesh->getVertexData(BitEngine::Mesh::VertexDataType::TextureCoord);
                BitEngine::Mesh::DataArray norms = mesh->getVertexData(BitEngine::Mesh::VertexDataType::Normals);
                for (int i = 0; i < indices.size; ++i) {
                    vertexBuffer.push<Shader3DSimple::Vertex>(Shader3DSimple::Vertex{
                        ((glm::vec3*)verts.data)[i],
                        glm::vec2(((glm::vec3*)texs.data)[i]),
                        ((glm::vec3*)norms.data)[i] });
                }
                Shader3DSimple::ShaderMesh& newNesh = (m_shaderMesh[mesh] = {});
                newNesh.setup((Shader3DSimple::Vertex*)vertexBuffer.base, indices.size, (u32*)indices.data, indices.size);
            }

            BitEngine::Material* material = cmd.data[end].material;
            if (m_shaderMaterials.find(material) == m_shaderMaterials.end()) {
                Shader3DSimple::Material3D& newMat = (m_shaderMaterials[material] = {});
                newMat.diffuse = material->getTexture(0);
                newMat.normal = material->getTexture(1);
            }
        }

        {
            BE_PROFILE_SCOPE("Setup render states");

            // Set up shader
            //glm::mat4 CameraMatrix = glm::lookAt(
            //        glm::vec3{ 50.f,0.f,300.f }, // camera pos
            //        glm::vec3{ 0.f,0.f,0.f },   // look to point
            //        glm::vec3{ 0.f,1.f,0.f }    // up vec
            //);

            //glm::mat4 projectionMatrix = glm::perspective(
            //    glm::radians(45.0f), // FOV
            //    16.0f / 9.0f,       // Ratio
            //    0.1f,              // Near plane
            //    900.0f            // Far plane
            //);

            m_shader.LoadProjectionMatrix(cmd.projection);
            m_shader.LoadViewMatrix(cmd.view);
            m_shader.Bind();


            // Set up GL states
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glCullFace(GL_BACK);
            glDepthMask(false);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }


        // Draw!
        {
            BE_PROFILE_SCOPE("Render calls");
            Shader3DSimple::BatchRenderer renderer;
            u32 otherEnd = 0;
            for (int i = 0; i < currentIndex; ++i) {
                u32 end = batchIndices[i];

                // Not sure where these will come from yet
                const Shader3DSimple::ShaderMesh& smesh = m_shaderMesh[cmd.data[end].mesh];
                const Shader3DSimple::Material3D& smat = m_shaderMaterials[cmd.data[end].material];

                renderer.draw(smesh, smat, (glm::mat4*) &matrixBuffer.base[otherEnd], end - otherEnd + 1);
                otherEnd = end;
            }
        }

    }

private:
    Shader3DSimple m_shader;
    std::map<BitEngine::Mesh*, Shader3DSimple::ShaderMesh> m_shaderMesh;
    std::map<BitEngine::Material*, Shader3DSimple::Material3D> m_shaderMaterials;
};

class GLRenderer {
public:

    void init(BitEngine::ResourceLoader* loader) {
        BE_PROFILE_FUNCTION();
        spriteRenderer.init(loader);
        modelsRenderer.init(loader);
    }

    void render(RenderQueue* queue) {
        BE_PROFILE_FUNCTION();

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
                BE_PROFILE_SCOPE("Render SCENE_3D_BATCH");
                Render3DBatchCommand& cmd = commands[i].data.batch3d;
                modelsRenderer.process(cmd);
                break;
            }

        }

    }


    GLSprite2DRenderer spriteRenderer;
    GLModelRenderer modelsRenderer;
};

