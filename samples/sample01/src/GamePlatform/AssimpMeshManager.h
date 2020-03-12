
#include <BitEngine/Core/TaskManager.h>
#include <BitEngine/Core/Resources/ResourceManager.h>
#include <BitEngine/Core/Resources/DevResourceLoader.h>
#include <BitEngine/Core/Mesh.h>

#include <assimp/scene.h>

using namespace BitEngine;


enum MeshLoadState {
    UNDEFINED,
    NOT_LOADED,
    LOADING,
    LOADED,
};

class AssimpMaterial : public BitEngine::Material {
public:

    RR<Texture> getTexture(int index) override {
        return textures[index];
    }

    int getTextureCount() override {
        return 4;
    }


    RR<Texture> textures[4];
};

class AssimpMesh : public BitEngine::Mesh {

public:
    AssimpMesh() : BitEngine::Mesh(nullptr) {

    }

    virtual DataArray getVertexData(VertexDataType type) override {
        switch (type) {
        case VertexDataType::Vertices:
            return DataArray{ mesh->mVertices, mesh->mNumVertices };
        case VertexDataType::TextureCoord:
            return DataArray{ mesh->mTextureCoords[0], mesh->mNumVertices };
        case VertexDataType::Normals:
            return DataArray{ mesh->mNormals, mesh->mNumVertices };
        }
        return {};
    }
    virtual DataArray getIndicesData(int index) override {
        return indices;
    }

    virtual int getType() const {
        return 0;
    }

    virtual Material* getMaterial() const {
        return material;
    }

    void setup(aiMesh* mesh, Material* material, DataArray indices) {
        this->mesh = mesh;
        this->material = material;
        this->indices = indices;

    }

    friend class AssimpMeshManager;
    friend class AssimpModelLoader;
private:

    aiMesh* mesh;
    DataArray indices;
    Material* material;
};

class AssimpModel : public BitEngine::Model
{

public:
    AssimpModel()
        : BitEngine::Model(nullptr), m_loaded(MeshLoadState::NOT_LOADED) {}
    AssimpModel(ResourceMeta* meta)
        : BitEngine::Model(meta), m_loaded(MeshLoadState::NOT_LOADED) {}

    u32 getMeshCount() override {
        if (m_loaded != LOADED) {
            return 0;
        }
        return scene->mNumMeshes;
    }

    Mesh* getMesh(int index) {
        return meshes[index];
    }

private:
    friend class AssimpModelLoader;
    friend class AssimpMeshManager;

    const aiScene* scene;
    MeshLoadState m_loaded;
    AssimpMesh* meshes[8];

};


class AssimpMeshManager : public BitEngine::ResourceManager {
public:
    AssimpMeshManager(BitEngine::TaskManager* tm) : taskManager(tm) {}
    ~AssimpMeshManager() {}

    virtual bool init() override;
    virtual void update() override;

    // Should release ALL resources
    virtual void shutdown() override;

    virtual void setResourceLoader(BitEngine::ResourceLoader* loader) override;


    virtual BitEngine::BaseResource* loadResource(BitEngine::ResourceMeta* meta, BitEngine::PropertyHolder* props) override;

    // Called when the given Resource Meta is not in use anymore
    // This will only be called after a previous call to loadResource() was made
    // Release the resource or not is up to the implementation
    // Usually cheap resources are kept in memory (may be released from drivers)
    virtual void resourceNotInUse(BitEngine::ResourceMeta* meta) override;

    virtual void reloadResource(BitEngine::BaseResource* resource) override;

    // Called after a while when the resource is not being used for some time.
    // After this call it's expected that most memory used by the resource is freed.
    // All resource references must still be valid, since we're just requesting the memory
    // for the resource to be released.
    virtual void resourceRelease(BitEngine::ResourceMeta* meta) override;

    // in bytes
    virtual ptrsize getCurrentRamUsage() const override;
    virtual u32 getCurrentGPUMemoryUsage() const override;
protected:
    friend class AssimpModelLoader;

    Mesh::DataArray generateIndices(AssimpMesh* assimpMesh) {
        Mesh::DataArray array = {};
        std::vector<u32> indices;
        for (u32 i = 0; i < assimpMesh->mesh->mNumFaces; i++)
        {
            aiFace face = assimpMesh->mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++) {
                indices.emplace_back(face.mIndices[j]);
            }
        }
        array.data = indices.data();
        array.size = indices.size();
        m_meshIndices[assimpMesh] = std::move(indices);
        return array;
    }

    AssimpMesh* setupMesh(const aiScene* scene, aiMesh* mesh) {
        AssimpMesh* assimpMesh = &m_meshes[m_meshCount];
        m_meshCount += 1;

        assimpMesh->mesh = mesh;
        AssimpMaterial& aMaterial = (m_materials[mesh] = AssimpMaterial());
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString str;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            aMaterial.textures[0] = m_loader->getResource<Texture>(str.C_Str());
        }

        if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString str;
            material->GetTexture(aiTextureType_NORMALS, 0, &str);
            aMaterial.textures[1] = m_loader->getResource<Texture>(str.C_Str());
        }
        assimpMesh->material = &aMaterial;

        Mesh::DataArray indices = generateIndices(assimpMesh);
        assimpMesh->indices = indices;
        return assimpMesh;
    }

private:
    void scheduleLoadingTasks(ResourceMeta* meta, AssimpModel* model);

    BitEngine::TaskManager* taskManager;
    BitEngine::DevResourceLoader* m_loader;
    BitEngine::ResourceIndexer<AssimpModel, 512> m_models;


    std::map<AssimpMesh*, std::vector<u32>> m_meshIndices;
    std::array<AssimpMesh, 512> m_meshes;
    u32 m_meshCount = 0;
    std::map<aiMesh*, AssimpMaterial> m_materials;
};