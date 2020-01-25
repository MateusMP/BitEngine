#include "GamePlatform/AssimpMeshManager.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace BitEngine;

enum MeshLoadState {
    UNDEFINED,
    NOT_LOADED,
    LOADING,
    LOADED,
};


struct AssimpMaterial {
    aiMaterial* material;
};



class AssimpModel
{
public:
    // Virtuals
    const Material* getMaterial(int index) const { return materials[index]; }
    const Mesh* getMesh(int index) const { return meshes[index]; }

    u32 getMeshCount() { return scene->mNumMeshes; }
    void* getMeshData(int meshIdx, MeshDataType type, ptrsize* size{
        switch (type) {
        case MeshDataType::Vertices:
            *size = scene->mMeshes[meshIdx]->mNumVertices;
            return scene->mMeshes[meshIdx]->mVertices;
        case MeshDataType::TextureCoord:
            *size = scene->mMeshes[meshIdx]->mNumVertices;
            return scene->mMeshes[meshIdx]->mTextureCoords[0];
        case MeshDataType::Normals:
            *size = scene->mMeshes[meshIdx]->mNumVertices;
            return scene->mMeshes[meshIdx]->mNormals;
        }
        }

private:
    friend class AssimpMeshLoad;
    friend class AssimpMeshManager;

    TightFixedVector<AssimpMesh, 32, false>* meshes;

    aiScene* scene;
    MeshLoadState m_loaded;
};


AssimpMesh::AssimpMesh(const std::string& baseDirectory) {
    m_baseDir = baseDirectory;
    LOG(EngineLog, BE_LOG_VERBOSE) << "MODEL CREATED: " << this;
}

AssimpMesh::~AssimpMesh()
{

}

///

class AssimpMeshLoad : public Task {
public:
    AssimpMeshLoad(AssimpMeshManager* mm, AssimpMesh* mesh, ResourceLoader::RawResourceTask dataTask)
        : Task(Task::TaskMode::REPEATING, Task::Affinity::MAIN), m_meshManager(mm), m_mesh(mesh), rawDataTask(dataTask)
    {

    }

    void run() override {

        ResourceLoader::DataRequest dr = rawDataTask->getData();
        if (dr.isLoaded()) {
            aiScene* scene = loadModel(dr.data, dr.size);
            process(scene);
        }

    }

    // Load model
    aiScene* loadModel(void* data, ptrsize size)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(data, size, aiProcess_Triangulate);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "ERROR::ASSIMP::" << importer.GetErrorString();
            return nullptr;
        }

        return scene;
    }

    void process(const aiScene* scene) {

        for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
            const aiMaterial* mat = scene->mMaterials[i];
            materials.emplace_back(createMaterial(tMng, mat));
        }

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        // Process all the node's meshes (if any)
        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        }
    }

    RR<Texture> getTexture(AssimpModel* model, aiTextureType type) {

        material = scene->mMaterials[mesh->mMaterialIndex]
        if (material->GetTextureCount(type) == 1)
        {
            aiString path;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

            mat->diffuse = tMng->getResource<Texture>(m_baseDir + std::string(path.C_Str()));
        }

        return mat;
    }


private:
    AssimpMesh* m_mesh;
    AssimpMeshManager* m_meshManager;
    ResourceLoader::RawResourceTask rawDataTask;
}


///



bool AssimpMeshManager::init() {
    return true;
}
void AssimpMeshManager::update() {

}

// Should release ALL resources
void AssimpMeshManager::shutdown() {

}

void AssimpMeshManager::setResourceLoader(ResourceLoader* loader) {

}

BaseResource* AssimpMeshManager::loadResource(ResourceMeta* meta, PropertyHolder* props) {

    Mesh* mesh = m_meshes.findResource(meta);

    if (mesh == nullptr)
    {
        u16 id = m_meshes.addResource(meta);
        mesh = m_meshes.getResourceAddress(id);

        new (mesh) Mesh(meta);
        {
            scheduleLoadingTasks(mesh);
            std::shared_ptr<AssimpMeshLoader> loadTask = std::make_shared<AssimpMeshLoader>(this, mesh);
            taskManager->addTask(loadTask);
        }

    }

    return shader;

}

void AssimpMeshManager::scheduleLoadingTasks(ResourceMeta* meta, AssimpMesh* mesh)
{
    BE_PROFILE_FUNCTION();
    mesh->m_loaded = GL2Texture::TextureLoadState::LOADING;
    ResourceLoader::RawResourceTask rawDataTask = loader->requestResourceData(mesh->getMeta());
    TaskPtr textureLoader = std::make_shared<AssimpMeshLoad>(this, mesh, rawDataTask);
    textureLoader->addDependency(rawDataTask);
    taskManager->addTask(textureLoader);
}

// Called when the given Resource Meta is not in use anymore
// This will only be called after a previous call to loadResource() was made
// Release the resource or not is up to the implementation
// Usually cheap resources are kept in memory (may be released from drivers)
void AssimpMeshManager::resourceNotInUse(ResourceMeta* meta) {

}

void AssimpMeshManager::reloadResource(BaseResource* resource) {

}

// Called after a while when the resource is not being used for some time.
// After this call it's expected that most memory used by the resource is freed.
// All resource references must still be valid, since we're just requesting the memory
// for the resource to be released.
void AssimpMeshManager::resourceRelease(ResourceMeta* meta) {

}

// in bytes
ptrsize AssimpMeshManager::getCurrentRamUsage() const {

}

u32 AssimpMeshManager::getCurrentGPUMemoryUsage() const {

}