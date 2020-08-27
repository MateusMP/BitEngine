#pragma once

#include <BitEngine/Core/Assert.h>
#include <Platform/opengl/GL2/GL2Driver.h>

#include "Shader3DSimple.h"


#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"

#include "Sprite2DBatch.h"

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

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
        if (cmd.batch.count <= 0) {
            return;
        }

        u8 _matrixBuffer[sizeof(glm::mat4) * 256];
        BitEngine::MemoryArena matrixBuffer;
        matrixBuffer.init(_matrixBuffer, sizeof(_matrixBuffer));
        
        // Sort for batching
        {
            BE_PROFILE_SCOPE("Sort models");
            std::sort(cmd.batch.data, cmd.batch.data + cmd.batch.count, [](const Model3D& a, const Model3D& b) {
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
            BitEngine::Material* lastMaterial = cmd.batch.data[0].material;
            BitEngine::Mesh* lastMesh = cmd.batch.data[0].mesh;
            for (u32 i = 0; i < cmd.batch.count; ++i) {
                const Model3D& model = cmd.batch.data[i];
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

        for (u32 i = 0; i < currentIndex; ++i) {
            u32 end = batchIndices[i];

            BitEngine::Mesh* mesh = cmd.batch.data[end].mesh;
            if (m_shaderMesh.find(mesh) == m_shaderMesh.end()) {
                vertexBuffer.init(_vertexBuffer, sizeof(_vertexBuffer));

                BitEngine::Mesh::DataArray indices = mesh->getIndicesData(0);
                BitEngine::Mesh::DataArray verts = mesh->getVertexData(BitEngine::Mesh::VertexDataType::Vertices);
                BitEngine::Mesh::DataArray texs = mesh->getVertexData(BitEngine::Mesh::VertexDataType::TextureCoord);
                BitEngine::Mesh::DataArray norms = mesh->getVertexData(BitEngine::Mesh::VertexDataType::Normals);
                BitEngine::Mesh::DataArray tangents = mesh->getVertexData(BitEngine::Mesh::VertexDataType::Tangent);
                for (int i = 0; i < indices.size; ++i) {
                    vertexBuffer.push<Shader3DSimple::Vertex>(Shader3DSimple::Vertex{
                        ((glm::vec3*)verts.data)[i],
                        glm::vec2(((glm::vec3*)texs.data)[i]),
                        ((glm::vec3*)norms.data)[i],
                        ((glm::vec3*)tangents.data)[i]
                        });
                }
                Shader3DSimple::ShaderMesh& newNesh = (m_shaderMesh[mesh] = {});
                newNesh.setup((Shader3DSimple::Vertex*)vertexBuffer.base, indices.size, (u32*)indices.data, indices.size);
            }

            BitEngine::Material* material = cmd.batch.data[end].material;
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
            m_shader.LoadLight(cmd.light.position, cmd.light.direction, glm::vec3(cmd.light.color.colors[0], cmd.light.color.colors[1], cmd.light.color.colors[2]));
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
            Shader3DSimple::BatchRenderer renderer = {};
            u32 otherEnd = 0;
            for (u32 i = 0; i < currentIndex; ++i) {
                u32 end = batchIndices[i];

                // Not sure where these will come from yet
                const Shader3DSimple::ShaderMesh& smesh = m_shaderMesh[cmd.batch.data[end].mesh];
                const Shader3DSimple::Material3D& smat = m_shaderMaterials[cmd.batch.data[end].material];

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
                SceneBeginCommand* cmd = static_cast<SceneBeginCommand *>(commands[i].data);
                // driver->clearBufferColor(nullptr, BitEngine::ColorRGBA(0.7f, 0.2f, 0.3f, 0.f));
                BitEngine::GLVideoDriver::clearBufferColor(nullptr, cmd->color);
                BitEngine::GLVideoDriver::clearBuffer(nullptr, BitEngine::BufferClearBitMask::COLOR_DEPTH);
            }
            break;
            case Command::SPRITE_BATCH_2D:
            {
                BE_PROFILE_SCOPE("Render SPRITE_BATCH_2D");
                spriteRenderer.process(*static_cast<Render2DBatchCommand*>(commands[i].data));
            }
            break;
            case Command::SCENE_3D_BATCH:
                BE_PROFILE_SCOPE("Render SCENE_3D_BATCH");
                Render3DBatchCommand* cmd = static_cast<Render3DBatchCommand*>(commands[i].data);
                modelsRenderer.process(*cmd);
                break;
            }

        }

    }


    GLSprite2DRenderer spriteRenderer;
    GLModelRenderer modelsRenderer;
};

