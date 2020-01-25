
#include <BitEngine/Core/TaskManager.h>
#include <BitEngine/Core/Resources/ResourceManager.h>
#include <BitEngine/Core/Mesh.h>

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


private:
    BitEngine::TaskManager* taskManager;
    BitEngine::ResourceLoader* loader;
    BitEngine::ResourceIndexer<BitEngine::Mesh, 1024> m_meshes;
    BitEngine::ResourceIndexer<AssimpMesh, 1024> m_meshes;
};