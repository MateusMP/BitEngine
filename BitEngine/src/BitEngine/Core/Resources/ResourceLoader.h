#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Bitengine/Core/Task.h"
#include "Bitengine/Core/Messenger.h"

#include "Bitengine/Core/Resources/ResourceIndexer.h"
#include "Bitengine/Core/Resources/ResourceProperties.h"

namespace BitEngine {

template<typename T>
class RR;

/***
 * Stores resource meta data.
 */
struct BE_API ResourceMeta
{
    ResourceMeta()
        : id(~0), references(0)
    {}

    const std::string toString() const {
        return "Resource: " + id;
    }

    u32 id; /// unique resource id
    nlohmann::json properties;

    u32 getReferences() const {
        return references;
    }

    std::string getNameId() const {
        return id + "";
    }

private:
    friend class ResourceLoader;
    u32 references;
};

/**
 * All resources types should come from this
 */
class BE_API BaseResource
{
public:
    // Base resource
    // d Owns the data from this vector
    BaseResource(ResourceMeta* _meta)
        : meta(_meta)
    {}

    u32 getResourceId() const {
        return meta->id;
    }

    ResourceMeta* getMeta() {
        return meta;
    }

protected:
    ResourceMeta* meta;
};

/**
 * Resource Loader interface
 * Used by the application to retrieve the final resource.
 */
class BE_API ResourceLoader
{
public:
    struct DataRequest
    {
        enum LoadState {
            NOT_LOADED,
            LS_LOADING,
            LS_LOADED,
            LS_ERROR,
        };
        DataRequest(DataRequest&& dr) noexcept
            : meta(dr.meta), loadState(dr.loadState), data(std::move(dr.data))
        {}

        DataRequest(ResourceMeta* _meta)
            : meta(_meta), loadState(NOT_LOADED)
        {}
        DataRequest& operator=(DataRequest&& other) {
            meta = other.meta;
            loadState = other.loadState;
            data = std::move(other.data);
            return *this;
        }

        bool isLoaded() {
            return loadState == LS_LOADED;
        }

        /*DataRequest& operator=(const DataRequest& other) {
        meta = other.meta;
        loadState = other.loadState;
        data = other.data;
        return *this;
        }*/

        ResourceMeta* meta;
        LoadState loadState;
        std::vector<char> data;
    };

    /**
     * This task is used when we need to load
     * file data from the file system.
     */
    class RawResourceLoaderTask : public Task
    {
    public:
        RawResourceLoaderTask(ResourceMeta* d)
            : Task(TaskMode::NONE, Affinity::BACKGROUND), dr(d)
        {}

        // Inherited via Task
        virtual void run() = 0;

        DataRequest& getData() {
            return dr;
        }

    protected:
        DataRequest dr;
    };
    typedef std::shared_ptr<RawResourceLoaderTask> RawResourceTask;
public:
    ResourceLoader() {}
    virtual ~ResourceLoader() {}

    virtual bool init() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;

    /**
     * TODO: This should not exist here. Loading from a file is not in this scope.
     * Blocking call
     * Read a file and map all indices
     * If any resource override would happen, they're skipped
     * and a warning is logged.
     * @param index The index file to load
     * @return true if succeeded
     */
    virtual bool loadIndex(const std::string& index) = 0;

    /**
     * Retrieve resource by name.
     * @param name the resource name.
     * @return a reference to the resource. It might not be fully loaded yet!
     */
    template<typename T>
    RR<T> getResource(const std::string& name) {
        T* resource = static_cast<T*>(loadResource(name));
        return RR<T>(resource, this);
    }

    template<typename T>
    RR<T> getResource(const u32& rid) {
        T* resource = static_cast<T*>(loadResource(rid));
        return RR<T>(resource, this);
    }

    template<typename T>
    RR<T> getResource(ResourceMeta* meta) {
        T* resource = static_cast<T*>(loadResource(meta));
        return RR<T>(resource, this);
    }

    /**
     * Force a resource to be reloaded.
     * @param resource the resource reference
     * @param wait if waiting the resource to be fully loaded is required (blocking call).
     */
    template<typename T>
    void reloadResource(RR<T>& resource, bool wait = false) {
        reloadResource(resource.get());
        if (wait) {
            waitForResource(resource.get());
        }
    }

    // Returns the request ID
    // Queue the request to be loaded by another thread
    //virtual u32 loadRequest(const std::string& path, BaseResource* into, ResourceManager* callback) = 0;
    //virtual bool reloadResource(u32 resourceID, ResourceManager* callback) = 0;

    /**
     * Wait all resources to be loaded.
     * This only considers resources that were previously requested.
     */
    virtual void waitForAll() = 0;

    /**
     * Wait a specifc resource to be loaded.
     * @param resource the resouce.
     */
    virtual void waitForResource(BaseResource* resource) = 0;

    /**
     * Release all resources in use. TAKE CARE WITH THIS!
     */
    virtual void releaseAll() = 0;

    /**
     * Returns whether a manager for specified type is available
     * Useful for safety checks on game start
     */
    virtual bool hasManagerForType(const std::string& type) = 0;

    /**
     * Copy of current loading tasks. Copy because this is updated by multiple threads.
     */
    virtual const std::map<ResourceMeta*, RawResourceTask> getPendingToLoad() = 0;

    virtual RawResourceTask loadRawData(ResourceMeta* meta) = 0;

protected:
    friend class ResourceManager;
    template<typename T> friend class RR;

    void incReference(BaseResource* r) {
        ++(r->getMeta()->references);
    }

    void decReference(BaseResource* r) {
        ResourceMeta* meta = r->getMeta();
        if (--(meta->references) == 0) {
            resourceNotInUse(meta); // TODO: Delegate this to be done by a task? Or later?
        }
    }

    /**
     * Called when the given Resource Meta is not in use anymore
     * This will only be called after a previous call to loadResource() was made
     * Release the resource or not is up to the implementation
     * Usually cheap resources are kept in memory (may be released from drivers)
     */
    virtual void resourceNotInUse(ResourceMeta* meta) = 0;

    /** Non blocking call
     * Retrieve the required resource by name
     * The resource is loaded on the first request
     * and stay loaded until not required anymore by any instance****
     * A temporary resource may be loaded, like a temporary texture or null sound.
     * To guarantee that the resource returned by this call is ready to use
     * follow this call by a waitForAll() or waitForResource(name)
     * @param name
     * @return
     */
    virtual BaseResource* loadResource(const std::string& name) = 0;

    virtual BaseResource* loadResource(const u32 rid) = 0;

    virtual BaseResource* loadResource(ResourceMeta* meta) = 0;

    // Will force a resource to be reloaded.
    virtual void reloadResource(BaseResource* resource) = 0;

    // Used internally by resource managers, to retrieve raw resource data
    // Like, texture image data, sound data, and others.
    // This will create a background task to load the resource.
    // The task shall be sent to the TaskManager before this function returns.
    virtual RawResourceTask requestResourceData(ResourceMeta* meta) = 0;
};



/**
 * Resource Reference class.
 * This is a Reference Counted reference to a given resource.
 */
template<typename T>
class BE_API RR
{
    static_assert(std::is_base_of<BaseResource, T>::value, "Not a resource class");

public:
    static RR<T> invalid() {
        return RR();
    }

    RR()
        : resource(nullptr), loader(nullptr)
    {}
    RR(T* r, ResourceLoader* l)
        : resource(r), loader(l)
    {
        incRef();
    }
    RR(RR&& r)
        : resource(r.resource), loader(r.loader)
    {}
    RR(const RR& r)
        : resource(r.resource), loader(r.loader)
    {
        incRef();
    }
    ~RR()
    {
        decRef();
    }
    RR& operator=(RR&& r)
    {
        resource = r.resource;
        loader = r.loader;
        return *this;
    }
    RR& operator=(const RR& r)
    {
        resource = r.resource;
        loader = r.loader;
        incRef();
        return *this;
    }
    T* operator->() {
        return resource;
    }
    const T* operator->() const {
        return resource;
    }

    bool operator !=(const RR& r) const
    {
        return resource == r.resource && loader == r.loader;
    }

    bool isValid() const {
        return resource != nullptr && loader != nullptr;
    }

    T* get() {
        return resource;
    }

    const T* get() const {
        return resource;
    }

    void invalidate() {
        decRef();
        resource = nullptr;
        loader = nullptr;
    }

private:
    void incRef() {
        if (resource == nullptr) return;
        loader->incReference(resource);
    }

    void decRef() {
        if (resource == nullptr) return;
        loader->decReference(resource);
    }

    T* resource;
    ResourceLoader* loader;
};


}
