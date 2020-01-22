#include "Shader3DSimple.h"

#include <assimp/material.h>
#include <bitengine/Core/Resources/ResourceManager.h>

//RendererVersion Shader3DSimple::useRenderer = NOT_DEFINED;

Shader3DSimple::Shader3DSimple()
    : shader(nullptr)
{

}

Shader3DSimple::~Shader3DSimple()
{
    delete shader;
}

Shader3DSimple::BatchRenderer* Shader3DSimple::CreateRenderer()
{
    return shader->CreateRenderer();
}

int Shader3DSimple::Init()
{
    Shader3DSimpleGL4* s = new Shader3DSimpleGL4();
    int error = s->Init();
    if (error == BE_NO_ERROR)
    {
        shader = s;
        LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Using Shader3DSimple GL4";
        return BE_NO_ERROR;
    }
    else {
        delete s;
        return error;
    }
}

void Shader3DSimple::LoadViewMatrix(const glm::mat4& matrix)
{
    shader->LoadViewMatrix(matrix);
}

void Shader3DSimple::LoadProjectionMatrix(const glm::mat4& matrix)
{
    shader->LoadProjectionMatrix(matrix);
}

void Shader3DSimple::Bind()
{
    shader->Bind();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////// MODEL //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Shader3DSimple::Model::Model(const std::string& baseDirectory) {
    m_baseDir = baseDirectory;
    LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "MODEL CREATED: " << this;
}

Shader3DSimple::Model::~Model()
{
    for (BitEngine::Material* m : materials)
        delete m;

    for (Mesh* m : meshes)
        delete m;
}

void Shader3DSimple::Model::process(BitEngine::ResourceLoader* tMng, const aiScene* scene) {

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* mat = scene->mMaterials[i];
        materials.emplace_back(createMaterial(tMng, mat));
    }

    processNode(scene->mRootNode, scene);
}

void Shader3DSimple::Model::processNode(aiNode* node, const aiScene* scene)
{
    // Process all the node's meshes (if any)
    for (GLuint i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        Mesh* newMesh = processMesh(mesh, scene);
        if (newMesh != nullptr) {
            meshes.push_back(newMesh);
        }
    }

    // Then do the same for each of its children
    for (GLuint i = 0; i < node->mNumChildren; i++)
    {
        this->processNode(node->mChildren[i], scene);
    }
}

typename Shader3DSimple::Mesh* Shader3DSimple::Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<BitEngine::Texture*> textures;

    for (GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        const glm::vec2 uv = (mesh->mTextureCoords[0]) ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0, 0);
        const glm::vec3 normals = (mesh->mNormals) ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0, 0, 0);

        vertices.emplace_back(
            glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
            uv,
            normals);
    }

    // Process indices
    for (GLuint i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Process material
    if (mesh->mMaterialIndex >= 0)
    {
        return new Mesh(mesh->mName.C_Str(), vertices, indices, static_cast<Shader3DSimple::Material3D*>(materials[mesh->mMaterialIndex]));
    }
    else
    {
        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Model has no material!";

        return nullptr;
    }
}

Shader3DSimple::Material3D* Shader3DSimple::Model::createMaterial(BitEngine::ResourceLoader* tMng, const aiMaterial* material) {
    Material3D* mat = new Material3D();

    if (material->GetTextureCount(aiTextureType_DIFFUSE) == 1)
    {
        aiString path;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

        mat->diffuse = tMng->getResource<BitEngine::Texture>(m_baseDir + std::string(path.C_Str()));
    }
    else
    {
        LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "unexpected number of diffuse textures: " << material->GetTextureCount(aiTextureType_NORMALS);
    }

    if (material->GetTextureCount(aiTextureType_NORMALS) == 1)
    {
        // aiTextureType_HEIGHT?
        aiString path;
        material->GetTexture(aiTextureType_NORMALS, 0, &path);

        mat->normal = tMng->getResource<BitEngine::Texture>(m_baseDir + std::string(path.C_Str()));
    }
    else
    {
        LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "unexpected number of normal textures: " << material->GetTextureCount(aiTextureType_NORMALS);
    }

    return mat;
}

////

// ******************* GL4 *******************

int Shader3DSimple::Shader3DSimpleGL4::Init()
{
    if (!BatchRendererGL4::CheckFunctions())
    {
        return SHADER_INIT_ERROR_NO_FUNCTIONS;
    }

    return BuildProgramFromFile(GL_VERTEX_SHADER, "data/shaders/shader3Dsimple.vtx",
        GL_FRAGMENT_SHADER, "data/shaders/shader3Dsimple.fgt");

}

void Shader3DSimple::Shader3DSimpleGL4::LoadViewMatrix(const glm::mat4& matrix)
{
    u_view = matrix;
}

void Shader3DSimple::Shader3DSimpleGL4::LoadProjectionMatrix(const glm::mat4& matrix)
{
    u_projection = matrix;
}

Shader3DSimple::BatchRenderer* Shader3DSimple::Shader3DSimpleGL4::CreateRenderer() {
    return new Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4();
}

void Shader3DSimple::Shader3DSimpleGL4::BindAttributes() {
    BindAttribute(ATTR_VERTEX_POS, "a_position");
    BindAttribute(ATTR_VERTEX_TEX, "a_textureUV");
    BindAttribute(ATTR_VERTEX_NORMAL, "a_normal");
    BindAttribute(ATTR_MODEL_MAT, "a_modelMatrix");

    //check_gl_error();
}

void Shader3DSimple::Shader3DSimpleGL4::RegisterUniforms() {
    LOAD_UNIFORM(u_projectionMatrixHDL, "u_projectionMatrix");
    LOAD_UNIFORM(u_viewMatrixHDL, "u_viewMatrix");
    LOAD_UNIFORM(u_diffuseHDL, "u_diffuse");
    LOAD_UNIFORM(u_normalHDL, "u_normal");
}

void Shader3DSimple::Shader3DSimpleGL4::OnBind() {
    connectTexture(u_diffuseHDL, DIFFUSE_TEXTURE_SLOT);
    connectTexture(u_normalHDL, NORMAL_TEXTURE_SLOT);

    loadMatrix4f(u_viewMatrixHDL, &(u_view[0][0]));
    loadMatrix4f(u_projectionMatrixHDL, &(u_projection[0][0]));
}


// ******************* GL 2 *******************

int Shader3DSimple::Shader3DSimpleGL2::Init()
{
    if (BatchRendererGL2::CheckFunctions())
    {
        return SHADER_INIT_ERROR_NO_FUNCTIONS;
    }

    return BuildProgramFromFile(GL_VERTEX_SHADER, "Shaders/shader3DsimpleGL2.vtx",
        GL_FRAGMENT_SHADER, "Shaders/shader3DsimpleGL2.fgt");
}

void Shader3DSimple::Shader3DSimpleGL2::LoadViewMatrix(const glm::mat4& matrix)
{
    u_view = matrix;
}

void Shader3DSimple::Shader3DSimpleGL2::LoadProjectionMatrix(const glm::mat4& matrix)
{
    u_projection = matrix;
}

void Shader3DSimple::Shader3DSimpleGL2::LoadIntanceModelMatrix(const glm::mat4& matrix)
{
    loadMatrix4f(u_modelMatrixHDL, &matrix[0][0]);
}

Shader3DSimple::BatchRenderer* Shader3DSimple::Shader3DSimpleGL2::CreateRenderer() {
    return new Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2(this);
}

void Shader3DSimple::Shader3DSimpleGL2::BindAttributes() {
    BindAttribute(ATTR_VERTEX_POS, "a_position");
    BindAttribute(ATTR_VERTEX_TEX, "a_textureUV");
    BindAttribute(ATTR_VERTEX_NORMAL, "a_normal");

    //check_gl_error();
}

void Shader3DSimple::Shader3DSimpleGL2::RegisterUniforms() {
    LOAD_UNIFORM(u_projectionMatrixHDL, "u_projectionMatrix");
    LOAD_UNIFORM(u_viewMatrixHDL, "u_viewMatrix");
    LOAD_UNIFORM(u_diffuseHDL, "u_diffuse");
    LOAD_UNIFORM(u_normalHDL, "u_normal");
    LOAD_UNIFORM(u_modelMatrixHDL, "u_modelMatrix");
}

void Shader3DSimple::Shader3DSimpleGL2::OnBind() {
    connectTexture(u_diffuseHDL, DIFFUSE_TEXTURE_SLOT);
    connectTexture(u_normalHDL, NORMAL_TEXTURE_SLOT);

    loadMatrix4f(u_viewMatrixHDL, &(u_view[0][0]));
    loadMatrix4f(u_projectionMatrixHDL, &(u_projection[0][0]));
}

////