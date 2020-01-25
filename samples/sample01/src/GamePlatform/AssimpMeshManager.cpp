#include "GamePlatform/AssimpMeshManager.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace BitEngine;

///

Assimp::Importer importer;

class AssimpModelLoader : public Task {
public:
    AssimpModelLoader(AssimpMeshManager* mm, DevResourceLoader* loader, AssimpModel* model, ResourceLoader::RawResourceTask dataTask)
        : Task(Task::TaskMode::NONE, Task::Affinity::MAIN), m_meshManager(mm), m_loader(loader), m_model(model), rawDataTask(dataTask)
    {

    }

    void run() override {

        const ResourceLoader::DataRequest& dr = rawDataTask->getData();
        if (dr.isLoaded()) {
            const aiScene* scene = loadModel(dr.data, dr.size);
            m_model->scene = scene;
            process(scene);
        }

    }

    // Load model
    const aiScene* loadModel(void* data, ptrsize size)
    {
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
            loadTextures(mat, aiTextureType_DIFFUSE);
            loadTextures(mat, aiTextureType_NORMALS);
            loadTextures(mat, aiTextureType_SPECULAR);
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

    void loadTextures(const aiMaterial* material, aiTextureType type) {

        for (int i = 0; i < material->GetTextureCount(type); ++i)
        {
            aiString path;
            material->GetTexture(type, i, &path);

            std::string strpath(path.C_Str());
            DevResourceMeta* meta = m_loader->findMeta(strpath);
            if (meta == nullptr) {
                DevResourceMeta* modelMeta = ((DevResourceMeta*)m_model->getMeta());
                DevResourceMeta* textureMeta = m_loader->createMeta(modelMeta->index, modelMeta->package, strpath, "TEXTURE", strpath, {});
                m_loader->getResource<Texture>(textureMeta);
            } else {
                m_loader->getResource<Texture>(meta);
            }
        }
    }


private:
    AssimpModel* m_model;
    AssimpMeshManager* m_meshManager;
    DevResourceLoader* m_loader;
    ResourceLoader::RawResourceTask rawDataTask;
};


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
    m_loader = (DevResourceLoader*)loader;
}

BaseResource* AssimpMeshManager::loadResource(ResourceMeta* meta, PropertyHolder* props) {

    AssimpModel* model = m_models.findResource(meta);

    if (model == nullptr)
    {
        u16 id = m_models.addResource(meta);
        model = m_models.getResourceAddress(id);

        new (model) AssimpModel(meta);
        {
            scheduleLoadingTasks(meta, model);
        }

    }

    return model;

}

void AssimpMeshManager::scheduleLoadingTasks(ResourceMeta* meta, AssimpModel* model)
{
    BE_PROFILE_FUNCTION();
    model->m_loaded = LOADING;
    ResourceLoader::RawResourceTask rawDataTask = m_loader->requestResourceData(model->getMeta());
    TaskPtr textureLoader = std::make_shared<AssimpModelLoader>(this, m_loader, model, rawDataTask);
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
    return 0;
}

u32 AssimpMeshManager::getCurrentGPUMemoryUsage() const {
    return 0;
}