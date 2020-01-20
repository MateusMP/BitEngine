#pragma once

// Read json index file

#include <nlohmann/json.hpp>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Memory.h"
#include "BitEngine/Core/Resources/ResourceManager.h"

#include <memory>
#include "BitEngine/Core/Math.h"
#include "BitEngine/Core/IO/File.h"
#include "BitEngine/Common/ThreadSafeQueue.h"


namespace BitEngine
{
class BaseResource;
class ResourceLoader;


struct BE_API DevResourceMeta : public ResourceMeta {

    DevResourceMeta(const std::string& pack, const std::string& resName)
        : package(pack), resourceName(resName)
    {}

    std::string package;
    std::string resourceName;
    std::string type;
    std::string filePath;

    nlohmann::json properties;
};


class FileLoaderTask : public ResourceLoader::RawResourceLoaderTask
{
public:
    FileLoaderTask(File* f, MemoryArena* arena, const std::string& filePath)
        : RawResourceLoaderTask(arena), path(filePath), file(f)
    {}

    void run() override
    {
        using namespace BitEngine;

        dr.loadState = ResourceLoader::DataRequest::LoadState::LS_LOADING;

        LOG(EngineLog, BE_LOG_VERBOSE) << "Data Loader: " << path;

        if (path.empty()) {
            throw "EMPTY PATH FOR RESOURCE";
        }

        if (loadFileToMemory(path, &dr))
        {
            file->data = dr.data;
            file->size = dr.size;
            file->ready = true;
            dr.loadState = ResourceLoader::DataRequest::LoadState::LS_LOADED;
        }
        else
        {
            LOG(EngineLog, BE_LOG_ERROR) << "Failed to open file: " << path;
            file->size = -1;
            file->ready = false;
            dr.loadState = ResourceLoader::DataRequest::LoadState::LS_ERROR;
        }
    }


    /**
     * Load file to memory
     * @param fname full path to file
     * @param out the file data
     * @return true if the file was loaded
     */
    static bool loadFileToMemory(const std::string& fname, ResourceLoader::DataRequest* into)
    {
        LOG(EngineLog, BE_LOG_VERBOSE) << "Loading resource index " << fname;
        std::ifstream file(fname, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return false;
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        into->data = into->arena->alloc(size);
        into->size = size;
        LOG(EngineLog, BE_LOG_VERBOSE) << fname << " size: " << size;

        file.read((char*)into->data, size);

        return file.gcount() == size;
    }

private:
    std::string path;
    File* file;
};

using FileLoadTask = std::shared_ptr<FileLoaderTask>;

class FolderFileManager : public ResourceManager
{
public:
    FolderFileManager(TaskManager* tm, MemoryArena& _arena) : taskManager(tm), arena(_arena) {

    }
    ~FolderFileManager() {

    }

    // Load textures that are ready to be sent to the GPU
    void update() override {
        BE_PROFILE_FUNCTION();
        std::pair<File*, FileLoadTask> task;

        std::vector< std::pair<File*, FileLoadTask> > retry;

        if (loadingFiles.tryPop(task)) {
            auto& dr = task.second->getData();
            if (dr.loadState == ResourceLoader::DataRequest::LoadState::LS_LOADING) {
                retry.emplace_back(task);
            } else {
                finishedLoading(task.first->getMeta());
            }
        }

        for (auto& it : retry) {
            loadingFiles.push(it);
        }
    }

    void shutdown() override {

    }

    const std::map<ResourceMeta*, FileLoadTask> getPendingToLoad() {
        waitingTasksMutex.lock();
        auto copy = waitingData;
        waitingTasksMutex.unlock();
        return copy;
    }

    void finishedLoading(ResourceMeta* meta) {
        std::lock_guard<std::mutex> lock(waitingTasksMutex);
        waitingData.erase(meta);
    }

    std::pair<ResourceLoader::RawResourceTask, File*> doload(DevResourceMeta* meta)
    {
        std::lock_guard<std::mutex> lock(waitingTasksMutex);
        auto it = waitingData.emplace(meta, nullptr);
        if (it.second)
        {
            File* found = files.findResource(meta);

            if (found == nullptr) {
                u16 id = files.addResource(meta);
                found = files.getResourceAddress(id);

                new (found) File(meta);

                FileLoadTask task = std::make_shared<FileLoaderTask>(found, &arena, meta->filePath);
                taskManager->addTask(task);
                it.first->second = task;

                loadingFiles.push({ found, task });
                return { task, found };
            }
            else {
                BE_ASSERT(false); // invalid code path
            }

        }
        std::shared_ptr<FileLoaderTask> ptr = it.first->second;
        return { std::static_pointer_cast<ResourceLoader::RawResourceLoaderTask, FileLoaderTask>(ptr), files.findResource(meta) };
    }

    BaseResource* loadResource(ResourceMeta* meta, PropertyHolder* props) override {
        return doload((DevResourceMeta*)meta).second;
    }

    ResourceLoader::RawResourceTask loadResource(DevResourceMeta* meta) {
        return doload((DevResourceMeta*)meta).first;
    }

    virtual bool init() override { return 1; }
    //
    virtual void setResourceLoader(ResourceLoader* loader) override {}

    // Called when the given Resource Meta is not in use anymore
    // This will only be called after a previous call to loadResource() was made
    // Release the resource or not is up to the implementation
    // Usually cheap resources are kept in memory (may be released from drivers)
    virtual void resourceNotInUse(ResourceMeta* meta) override {}

    virtual void reloadResource(BaseResource* resource) override {}

    // Called after a while when the resource is not being used for some time.
    // After this call it's expected that most memory used by the resource is freed.
    // All resource references must still be valid, since we're just requesting the memory
    // for the resource to be released.
    virtual void resourceRelease(ResourceMeta* meta) override {}

    // in bytes
    virtual ptrsize getCurrentRamUsage() const override { return 0; }
    virtual u32 getCurrentGPUMemoryUsage() const override { return 0; }

private:

    // Members
    MemoryArena& arena;
    TaskManager* taskManager;

    ResourceIndexer<File, 1024> files;
    ThreadSafeQueue<std::pair<File*, FileLoadTask>> loadingFiles;

    std::mutex waitingTasksMutex;
    std::map<ResourceMeta*, FileLoadTask> waitingData; // the resources that are waiting the raw data to be loaded
};




/**
 * Resource Loader implementation
 * This loader loads files from the standard file system.
 * All resources are indexed in a json file.
 */
class DevResourceLoader : public ResourceLoader
{
    friend class DevPropHolder;
public:
    DevResourceLoader(TaskManager* taskManager, MemoryArena &arena);
    ~DevResourceLoader();

    bool init() override;
    virtual void update() override;

    // Inherited via ResourceLoader
    virtual void shutdown() override;

    void registerResourceManager(const std::string& resourceType, ResourceManager* manager) {
        taskManager->verifyMainThread();
        BE_ASSERT(manager != nullptr);
        manager->setResourceLoader(this);
        managers.emplace_back(manager);
        managersMap[resourceType] = ManagerInfo{ manager };
    }

    virtual bool loadIndex(const std::string& indexFilename) override;

    void convertIndexesToProd();

    DevResourceMeta* findMeta(const std::string& name);
    
    virtual bool hasManagerForType(const std::string& resourceType) override;

    const std::map<ResourceMeta*, ResourceLoader::RawResourceTask> getPendingToLoad() override {
        // return folderFileManager->getPendingToLoad();
        return {};
    }

    ResourceLoader::RawResourceTask requestResourceData(ResourceMeta* meta) override {
        DevResourceMeta* dmeta = static_cast<DevResourceMeta*>(meta);
        return folderFileManager.loadResource(dmeta);
    }

protected:
    // Retrieve 
    virtual BaseResource* loadResource(const u32 rid) override;
    virtual BaseResource* loadResource(ResourceMeta* meta) override;
    virtual BaseResource* loadResource(const std::string& meta) override;
    virtual void reloadResource(BaseResource* resource) override;

    virtual void releaseAll() override;
    virtual void resourceNotInUse(ResourceMeta* meta) override;

    virtual void waitForAll() override;
    virtual void waitForResource(BaseResource* resource) override;

    // Returns nullptr if meta conflicts and allowOverride == false
    DevResourceMeta* addResourceMeta(const DevResourceMeta& meta, bool allowOverride);

    
    friend class DevLoaderTask;

private:
    struct LoadedIndex {
        std::string name;
        std::string basefilepath;
        nlohmann::json data;
        std::vector<DevResourceMeta> metas;
    };

    using ResourceType = std::string;
    
    bool isManagerForTypeAvailable(const std::string& type);

    void loadPackages(LoadedIndex* index, bool allowOverride);
    LoadedIndex* findIndexByName(const std::string& string);

    // Holds the managers
    std::vector<ResourceManager*> managers;

    struct ManagerInfo {
        ResourceManager* mngr;
    };
    std::unordered_map<ResourceType, ManagerInfo> managersMap;

    std::vector<LoadedIndex> resourceMetaIndexes;
    std::unordered_map<std::string, DevResourceMeta*> byName;
    std::unordered_map<u32, DevResourceMeta*> byId;
    
    TaskManager* taskManager;
    FolderFileManager folderFileManager;
};

class DevPropHolder : public PropertyHolder {
public:
    /// MemoryArena required for loading complex objects, because we need to allocate
    /// temporary DevPropHolders. The arena should be temporary and is expected to be
    /// freed once DevPropHolder goes out of scope.
    DevPropHolder(DevResourceLoader* l, const nlohmann::json& p, MemoryArena& tmpMem) 
        : loader(l), properties(p), tempArena(tmpMem) {

    }
    DevResourceLoader* getLoader() override {
        return loader;
    }

    ptrsize getPropertyListSize(const char* name) override {
        return properties[name].size();
    }

    template<typename T>
    T valueOrDefault(const char* name, T def) {
        const auto& it = properties.find(name);
        if (it != properties.end()) {
            return it->get<T>();
        }
        return def;
    }

    void _read(const char* name, u32* into) override {
        *into = valueOrDefault<u32>(name, 0);
    }
    void _read(const char* name, s32* into) override {
        *into = valueOrDefault<s32>(name, 0);
    }
    void _read(const char* name, float* into) override {
        *into = valueOrDefault<float>(name, 0);
    }
    void _read(const char* name, double* into) override {
        *into = valueOrDefault<double>(name, 0);
    }
    void _read(const char* name, std::string* into) override {
        *into = valueOrDefault<std::string>(name, "");
    }
    void _read(const char* name, ResourceMeta** into) override {
        if (properties.contains(name)) {
            const std::string& res = properties[name].get_ref<const std::string&>();
            *into = loader->findMeta(res);
        }
        else {
            *into = nullptr;
        }
    }
    void _read(const char* name, BaseResource** into) override {
        const auto& it = properties.find(name);
        if (it != properties.end()) {
            const std::string& res = it->get_ref<const std::string&>();
            DevResourceMeta* meta = loader->findMeta(res);
            *into = loader->loadResource(meta);
        }
    }
    void _read(const char* name, Vec3* into) override {
        const nlohmann::json& props = properties[name];
        (*into)[0] = props[0].get<float>();
        (*into)[1] = props[1].get<float>();
        (*into)[2] = props[2].get<float>();
    }
    void _read(const char* name, Vec4* into) override {
        const nlohmann::json& props = properties[name];
        (*into)[0] = props[0].get<float>();
        (*into)[1] = props[1].get<float>();
        (*into)[2] = props[2].get<float>();
        (*into)[3] = props[3].get<float>();
    }
protected:
    PropertyHolder* getReaderObject(const char* name) override {
        return tempArena.push<DevPropHolder>(loader, properties[name], tempArena);
    }
    PropertyHolder* getReaderObjectFromList(const char* name, ptrsize index) override {
        return tempArena.push<DevPropHolder>(loader, properties[name][index], tempArena);
    }

    DevResourceLoader* loader;
    const nlohmann::json& properties;
    MemoryArena& tempArena;
};

}
