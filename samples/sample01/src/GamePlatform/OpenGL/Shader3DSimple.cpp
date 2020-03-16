#include "Shader3DSimple.h"

#include <assimp/material.h>
#include <BitEngine/Core/Resources/ResourceManager.h>

//RendererVersion Shader3DSimple::useRenderer = NOT_DEFINED;

Shader3DSimple::Shader3DSimple()
{

}

Shader3DSimple::~Shader3DSimple()
{
}

int Shader3DSimple::Init()
{
    if (!BatchRenderer::CheckFunctions())
    {
        return SHADER_INIT_ERROR_NO_FUNCTIONS;
    }

    return BuildProgramFromFile(GL_VERTEX_SHADER, "../data/shaders/shader3Dsimple.vtx",
        GL_FRAGMENT_SHADER, "../data/shaders/shader3Dsimple.fgt");

}

void Shader3DSimple::LoadViewMatrix(const glm::mat4& matrix)
{
    u_view = matrix;
}

void Shader3DSimple::LoadProjectionMatrix(const glm::mat4& matrix)
{
    u_projection = matrix;
}

void Shader3DSimple::LoadLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color)
{
    u_light.position = position;
    u_light.direction = direction;
    u_light.color = color;
}

void Shader3DSimple::BindAttributes() {
    BindAttribute(ATTR_VERTEX_POS, "a_position");
    BindAttribute(ATTR_VERTEX_TEX, "a_textureUV");
    BindAttribute(ATTR_VERTEX_NORMAL, "a_normal");
    BindAttribute(ATTR_VERTEX_TANGENT, "a_tangent");
    BindAttribute(ATTR_MODEL_MAT, "a_modelMatrix");

    //check_gl_error();
}

void Shader3DSimple::RegisterUniforms() {
    LOAD_UNIFORM(u_projectionMatrixHDL, "u_projectionMatrix");
    LOAD_UNIFORM(u_viewMatrixHDL, "u_viewMatrix");
    LOAD_UNIFORM(u_diffuseHDL, "u_diffuse");
    LOAD_UNIFORM(u_normalHDL, "u_normal");
    LOAD_UNIFORM(u_light_posHDL, "u_light.pos");
    LOAD_UNIFORM(u_light_dirHDL, "u_light.dir");
    LOAD_UNIFORM(u_light_colorHDL, "u_light.color");
}

void Shader3DSimple::OnBind() {
    connectTexture(u_diffuseHDL, DIFFUSE_TEXTURE_SLOT);
    connectTexture(u_normalHDL, NORMAL_TEXTURE_SLOT);

    loadMatrix4f(u_viewMatrixHDL, &(u_view[0][0]));
    loadMatrix4f(u_projectionMatrixHDL, &(u_projection[0][0]));

    loadVector3f(u_light_posHDL, 1, (float*)&u_light.position);
    loadVector3f(u_light_dirHDL, 1, (float*)&u_light.direction);
    loadVector3f(u_light_colorHDL, 1, (float*)&u_light.color);
}