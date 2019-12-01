#pragma once

// Read json index file

#include <nlohmann/json.hpp>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Memory.h"
#include "BitEngine/Core/Resources/ResourceManager.h"

#include "BitEngine/Core/Math.h"


namespace BitEngine
{
struct BE_API DevResourceMeta : public ResourceMeta {
    DevResourceMeta(const std::string& pack)
        : package(pack)
    {}

    std::string package;
    std::string resourceName;
    std::string type;
    std::string filePath; // In case the resource is associated with a file
};

/**
 * Resource Loader implementation
 * This loader loads files from the standard file system.
 * All resources are indexed in a json file.
 */
class DevResourceLoader : public ResourceLoader
{
public:


    struct Serializer {
        template<typename T>
        void write(const char* name, const T value) {}
    };

    struct Deserializer {
        template<typename T>
        void read(const char* name, T* value) {
            *value = properties[name].get<T>();
        }

        template<typename T>
        void read(const char* name, RR<T>* value) {
            const std::string& ref = properties[name].get_ref<const std::string&>();
            *value = loader->getResource<T>(ref);
        }

        template<u32 K, typename P, glm::qualifier X>
        void read(const char* name, glm::vec<K, P, X>* value) {
            const std::vector<P>& values = properties[name];
            for (u32 i = 0; i < K; ++i) {
                (*value)[i] = values[i];
            }
        }

        DevResourceLoader* loader;
        const nlohmann::json& properties;
    };

    using SerializerCall = void(*)(Serializer*, const BaseResource*);
    using DeserializerCall = void(*)(Deserializer*, BaseResource*);
    using PropertyHandler = void(*)(DevResourceLoader* , nlohmann::json& props, ResourceManager*, BaseResource*);
    using PropertyToProdConverter = void(*)(Serializer*, ResourceManager*, nlohmann::json& props, BaseResource*);

    // Returns the full directory where the file for this meta is.
    static std::string getPackagePath(const ResourceMeta* meta);

    /**
     * Load file to memory
     * @param fname full path to file
     * @param out the file data
     * @return true if the file was loaded
     */
    static bool loadFileToMemory(const std::string& fname, std::vector<char>& out);


    DevResourceLoader(TaskManager* taskManager);
    ~DevResourceLoader();

    bool init() override;
    virtual void update() override;

    // Inherited via ResourceLoader
    virtual void shutdown() override;

    template<typename T, typename Manager>
    void registerResourceManager(const std::string& resourceType, Manager* manager) {
        taskManager->verifyMainThread();
        BE_ASSERT(manager != nullptr);
        manager->setResourceLoader(this);
        managers.emplace_back(manager);
        managersMap[resourceType] = ManagerInfo{ manager, 
            T::serialize,
            T::deserialize,
            Manager::readJsonProperties,
            Manager::jsonPropertiesToProd,
        };
    }

    virtual bool loadIndex(const std::string& indexFilename) override;

    void convertIndexesToProd();

    DevResourceMeta* findMeta(const std::string& name);

    ResourceMeta* includeMeta(const std::string& package, const std::string& resourceName,
        const std::string& type);

    virtual bool hasManagerForType(const std::string& resourceType) override;

    const std::map<ResourceMeta*, ResourceLoader::RawResourceTask> getPendingToLoad() override {
        waitingTasksMutex.lock();
        auto copy = waitingData;
        waitingTasksMutex.unlock();
        return copy;
    }

    ResourceLoader::RawResourceTask loadRawData(ResourceMeta* meta) override {
        std::lock_guard<std::mutex> lock(waitingTasksMutex);
        auto it = waitingData.emplace(meta, nullptr);
        if (it.second) {
            it.first->second = requestResourceData(meta);
        }
        return it.first->second;
    }

protected:
    // Retrieve 
    virtual BaseResource* loadResource(const std::string& name) override;
    virtual BaseResource* loadResource(const u32 rid) override;
    virtual BaseResource* loadResource(ResourceMeta* meta) override;
    virtual void reloadResource(BaseResource* resource) override;

    virtual void releaseAll() override;
    virtual void resourceNotInUse(ResourceMeta* meta) override;

    virtual void waitForAll() override;
    virtual void waitForResource(BaseResource* resource) override;

    // Returns nullptr if meta conflicts and allowOverride == false
    DevResourceMeta* addResourceMeta(const DevResourceMeta& meta, bool allowOverride);

    
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

    void loadPackages(LoadedIndex* index, bool allowOverride);
    LoadedIndex* findIndexByName(const std::string& string);

    // Holds the managers
    std::mutex waitingTasksMutex;
    std::vector<ResourceManager*> managers;

    struct ManagerInfo {
        ResourceManager* mngr;
        SerializerCall serializer;
        DeserializerCall deserializer;
        PropertyHandler propHandler;
        PropertyToProdConverter propToProd;
    };
    std::unordered_map<std::string, ManagerInfo> managersMap;

    std::vector<DevResourceMeta> resourceMeta;
    std::vector<LoadedIndex> resourceMetaIndexes;
    std::unordered_map<std::string, u32> byName;
    std::map<ResourceMeta*, ResourceLoader::RawResourceTask> waitingData; // the resources that are waiting the raw data to be loaded

    TaskManager* taskManager;
};

}
