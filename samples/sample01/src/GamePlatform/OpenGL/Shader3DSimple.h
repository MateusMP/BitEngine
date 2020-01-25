#pragma once

#include <map>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <BitEngine/bitengine.h>
#include <BitEngine/Core/Logger.h>
#include <Platform/opengl/OpenGL.h>
#include <Platform/opengl/VertexArrayObject.h>
#include <Platform/opengl/ShaderProgram.h>


// GL4/3
DECLARE_VERTEXDATA(Simple3Dmatrix_VD, 4)
ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0)
END_ATTRIBUTES()

DECLARE_DATA_STRUCTURE()
ADD_MEMBER(glm::mat4, matrix)
END_VERTEX_DATA()

class Shader3DSimple : public BitEngine::ShaderProgram
{

public:
    class BatchRenderer;

    static const int DIFFUSE_TEXTURE_SLOT = 0;
    static const int NORMAL_TEXTURE_SLOT = 1;

    static const u32 ATTR_VERTEX_POS = 0;	// 3 floats	by vertex
    static const u32 ATTR_VERTEX_TEX = 1;	// 2 floats	by vertex
    static const u32 ATTR_VERTEX_NORMAL = 2;	// 3 floats by vertex
    static const u32 ATTR_MODEL_MAT = 3;		// 16 floats by instance

    static const u32 NUM_VBOS = 3;
    static const u32 VBO_INDEX = 0;
    static const u32 VBO_VERTEXDATA = 1;
    static const u32 VBO_MODELMAT = 2;

    static int GetID() {
        return 1;
    }

    Shader3DSimple();
    ~Shader3DSimple();

    int Init() override;

    void BindAttributes() override;

    /// VIRTUAL
    void RegisterUniforms() override;

    /// Virtual
    void OnBind() override;

    void LoadViewMatrix(const glm::mat4& matrix);
    void LoadProjectionMatrix(const glm::mat4& matrix);


public:
    // Shader classes
    class Material3D : public BitEngine::Material
    {
    public:
        BitEngine::RR<BitEngine::Texture> diffuse;
        BitEngine::RR<BitEngine::Texture> normal;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    struct ShaderMesh
    {
        void setup(Vertex* vertices, ptrsize vertexCount, u32* indices, ptrsize indexSize) {
            
            // Create buffers
            glGenVertexArrays(1, &vao);
            glGenBuffers(NUM_VBOS, vbo);

            // Load index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO_INDEX]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(GLuint), indices, GL_STATIC_DRAW);

            // Vertex data
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_VERTEXDATA]);
            glEnableVertexAttribArray(ATTR_VERTEX_POS);
            glVertexAttribPointer(ATTR_VERTEX_POS, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)0);
            glVertexAttribDivisor(ATTR_VERTEX_POS, 0);

            glEnableVertexAttribArray(ATTR_VERTEX_TEX);
            glVertexAttribPointer(ATTR_VERTEX_TEX, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(3 * sizeof(GLfloat)));
            glVertexAttribDivisor(ATTR_VERTEX_TEX, 0);

            glEnableVertexAttribArray(ATTR_VERTEX_NORMAL);
            glVertexAttribPointer(ATTR_VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(5 * sizeof(GLfloat)));
            glVertexAttribDivisor(ATTR_VERTEX_NORMAL, 0);

            // Load vertex data

            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

            // Setup VBO for data, but do not bind data yet, since it is dynamic
            glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_MODELMAT]);
            for (int i = 0; i < VERTEX_MATRIX4_ATTIBUTE_SIZE; ++i) {
                glEnableVertexAttribArray(ATTR_MODEL_MAT + i);
                glVertexAttribPointer(ATTR_MODEL_MAT + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(GLfloat) * 4 * i));
                glVertexAttribDivisor(ATTR_MODEL_MAT + i, 1);
            }

            glBindVertexArray(0);

            //check_gl_error();
        }

        void destroy() {
            glDeleteBuffers(NUM_VBOS, vbo);
            glDeleteVertexArrays(1, &vao);
        }

        GLuint vao;
        GLuint vbo[NUM_VBOS];
        u32 numIndices;
    };

    class BatchRenderer
    {
    public:
        static bool CheckFunctions()
        {
            if (glDrawElementsInstancedBaseInstance == nullptr)
                return false;

            return true;
        }

        void draw(const ShaderMesh& mesh, const Material3D& material , const glm::mat4* modelMats, ptrsize totalTransforms) {

            // Load transform matrices
            glBindVertexArray(mesh.vao);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[VBO_MODELMAT]);
            glBufferData(GL_ARRAY_BUFFER, totalTransforms * sizeof(glm::mat4), modelMats, GL_DYNAMIC_DRAW);

            /*
            if (r.transparent) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }*/

            // Bind DIFFUSE
            const GLuint diffuse = material.diffuse->getTextureID();
            if (lastDiffuse != diffuse) {
                glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_SLOT);
                glBindTexture(GL_TEXTURE_2D, diffuse);
                lastDiffuse = diffuse;
            }

            // Bind NORMAL
            const GLuint normal = material.normal->getTextureID();
            if (lastNormal != normal) {
                glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_SLOT);
                glBindTexture(GL_TEXTURE_2D, normal);
                lastNormal = normal;
            }

            // DRAW
            glDrawElementsInstancedBaseInstance(GL_TRIANGLES, r.mesh->getNumIndices(), GL_UNSIGNED_INT, 0, r.nItems, r.offset);

            /*
            if (r.transparent) {
                glDisable(GL_BLEND);
            }*/
        }

        GLuint lastDiffuse;
        GLuint lastNormal;

    };


protected:
    // Locations
    s32 u_projectionMatrixHDL;
    s32 u_viewMatrixHDL;
    s32 u_diffuseHDL;
    s32 u_normalHDL;

    // Data
    glm::mat4 u_projection;
    glm::mat4 u_view;
};
