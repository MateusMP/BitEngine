#pragma once

// Read json index file

#include <nlohmann/json.hpp>

#include "bitengine/Core/TaskManager.h"
#include "bitengine/Core/Memory.h"
#include "bitengine/Core/Resources/ResourceManager.h"


namespace BitEngine
{
struct BE_API DevResourceMeta : public ResourceMeta {
    std::string filePath; // In case the resource is associated with a file

    DevResourceMeta(const std::string& pack)
        : ResourceMeta(pack)
    {}
};

struct BE_API DevResourcePropertyRef : public ResourceProperty
{
    DevResourcePropertyRef(nlohmann::json& json)
        : properties(json)
    {
    }
    ~DevResourcePropertyRef() {
        //LOG(EngineLog, BE_LOG_VERBOSE) << this << " was reference removed!";
    }

    ResourcePropertyContainer getProperty(const std::string& str) override
    {
        auto it = properties.find(str);
        if (it == properties.end())
        {
            return ResourcePropertyContainer(nullptr);
        }
        else
        {
            return ResourcePropertyContainer(new DevResourcePropertyRef(*it));
        }
    }

    virtual ResourcePropertyContainer getProperty(int index) override {
        return ResourcePropertyContainer(new DevResourcePropertyRef(properties[index]));
    }

    virtual const std::string& getValueString() const override {
        return properties.get_ref<std::string&>();
    }
    virtual double getValueDouble() const override {
        return properties.get<double>();
    }
    int getValueInt() const override {
        return properties.get<int>();
    }
    virtual int getNumberOfProperties() const override {
        return properties.size();
    }

    virtual ResourcePropertyContainer operator[](const std::string& str) override { return this->getProperty(str); }

private:
    nlohmann::json& properties;
};

/**
 * Resource Loader implementation
 * This loader loads files from the standard file system.
 * All resources are indexed in a json file.
 */
class BE_API DevResourceLoader : public ResourceLoader
{
public:
    // Returns the full directory where the file for this meta is.
    static std::string getPackagePath(const ResourceMeta* meta);

    /**
     * Load file to memory
     * @param fname full path to file
     * @param out the file data
     * @return true if the file was loaded
     */
    static bool loadFileToMemory(const std::string& fname, std::vector<char>& out);


    DevResourceLoader(MemoryArena& memoryArena, Messenger* ge, TaskManager* taskManager);
    ~DevResourceLoader();

    bool init() override;
    virtual void update() override;

    // Inherited via ResourceLoader
    virtual void shutdown() override;
    virtual void registerResourceManager(const std::string& resourceType, ResourceManager* manager) override;

    virtual bool loadIndex(const std::string& indexFilename) override;

    virtual ResourceMeta* findMeta(const std::string& name) override;

    BitEngine::ResourceMeta* includeMeta(const std::string& package, const std::string& resourceName,
        const std::string& type, ResourcePropertyContainer properties) override;

    virtual bool hasManagerForType(const std::string& resourceType) override;

    const std::map<ResourceMeta*, ResourceLoader::RawResourceTask> getPendingToLoad() override {
        waitingTasksMutex.lock();
        auto copy = waitingData;
        waitingTasksMutex.unlock();
        return copy;
    }

protected:
    // Retrieve 
    virtual BaseResource* loadResource(const std::string& name) override;
    virtual void reloadResource(BaseResource* resource) override;

    virtual void releaseAll() override;
    virtual void resourceNotInUse(ResourceMeta* meta) override;

    virtual void waitForAll() override;
    virtual void waitForResource(BaseResource* resource) override;

    // Returns nullptr if meta conflicts and allowOverride == false
    DevResourceMeta* addResourceMeta(const DevResourceMeta& meta, bool allowOverride);

    ResourceLoader::RawResourceTask loadRawData(ResourceMeta* meta) override {
        std::lock_guard<std::mutex> lock(waitingTasksMutex);
        auto it = waitingData.emplace(meta, nullptr);
        if (it.second) {
            it.first->second = requestResourceData(meta);
        }
        return it.first->second;
    }
    
    friend class DevLoaderTask;
    void finishedLoading(ResourceMeta* meta) {
        std::lock_guard<std::mutex> lock(waitingTasksMutex);
        waitingData.erase(meta);
    }

private:
    struct LoadedIndex {
        std::string name;
        nlohmann::json data;
        std::vector<DevResourceMeta*> metas;
    };
    RawResourceTask requestResourceData(ResourceMeta* meta) override;

    bool isManagerForTypeAvailable(const std::string& type);
    BitEngine::BaseResource* getResourceFromManager(ResourceMeta* meta);

    void loadPackages(LoadedIndex* index, bool allowOverride);
    LoadedIndex* findIndexByName(const std::string& string);

    // Holds the managers
    std::mutex waitingTasksMutex;
    std::vector<ResourceManager*> managers;
    std::unordered_map<std::string, ResourceManager*> managersMap;

    std::vector<DevResourceMeta> resourceMeta;
    std::vector<LoadedIndex> resourceMetaIndexes;
    std::unordered_map<std::string, u32> byName;
    std::map<ResourceMeta*, ResourceLoader::RawResourceTask> waitingData; // the resources that are waiting the raw data to be loaded

    TaskManager* taskManager;

    MemoryArena& memoryArena;
};

}
