#pragma once

// Read json index file

#include <nlohmann/json.hpp>

#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Memory.h"
#include "BitEngine/Core/Resources/ResourceManager.h"

#include "BitEngine/Core/Math.h"


namespace BitEngine
{
struct BE_API ProdResourceMeta : public ResourceMeta {
    ProdResourceMeta()
        : ResourceMeta()
    {}
};

/**
 * Resource Loader implementation
 * This loader loads files from the standard file system.
 * All resources are indexed in a json file.
 */
class ProdResourceLoader : public ResourceLoader
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
            u32 resourceId = properties[name].get<u32>();
            *value = loader->getResource<T>(resourceId);
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

    ProdResourceLoader(TaskManager* taskManager);
    ~ProdResourceLoader();

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
        };
    }

    virtual bool loadIndex(const std::string& indexFilename) override;

    DevResourceMeta* findMeta(const std::string& name);

    const std::map<ResourceMeta*, ResourceLoader::RawResourceTask> getPendingToLoad() override {
        waitingTasksMutex.lock();
        auto copy = waitingData;
        waitingTasksMutex.unlock();
        return copy;
    }

protected:
    // Retrieve 
    virtual BaseResource* loadResource(const std::string& name) override {}
    virtual BaseResource* loadResource(const u32 rid) override {

    }

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

    void loadPackages(LoadedIndex* index, bool allowOverride);
    LoadedIndex* findIndexByName(const std::string& string);

    // Holds the managers
    std::mutex waitingTasksMutex;
    std::vector<ResourceManager*> managers;

    struct ManagerInfo {
        ResourceManager* mngr;
        SerializerCall serializer;
        DeserializerCall deserializer;
    };
    std::unordered_map<std::string, ManagerInfo> managersMap;

    std::vector<DevResourceMeta> resourceMeta;
    std::vector<LoadedIndex> resourceMetaIndexes;
    std::unordered_map<std::string, u32> byName;
    std::map<ResourceMeta*, ResourceLoader::RawResourceTask> waitingData; // the resources that are waiting the raw data to be loaded

    TaskManager* taskManager;
};

}
