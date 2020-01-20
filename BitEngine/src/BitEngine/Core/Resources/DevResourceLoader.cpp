#include <fstream>
#include <filesystem>

#include "BitEngine/Core/Resources/DevResourceLoader.h"

#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/Task.h"
#include "BitEngine/Core/TaskManager.h"
#include "BitEngine/Core/Graphics/Sprite.h"

namespace BitEngine {


std::string getPackagePath(const std::string& package, const std::string& resourceName)
{
    return "data/" + package + "/" + resourceName;
}

std::string getPackagePath(const DevResourceMeta* meta)
{
    return getPackagePath(meta->package, meta->resourceName);
}

//

DevResourceLoader::DevResourceLoader(TaskManager* tm, MemoryArena &arena)
    : taskManager(tm), folderFileManager(tm, arena)
{
    resourceMetaIndexes.reserve(8);
}

DevResourceLoader::~DevResourceLoader()
{
    shutdown();
}

bool DevResourceLoader::init()
{
    taskManager->verifyMainThread();
    registerResourceManager("FILE", &folderFileManager);
    for (ResourceManager* it : managers)
    {
        it->init();
    }
    return true;
}

void DevResourceLoader::update()
{
    taskManager->verifyMainThread();
    for (ResourceManager* it : managers)
    {
        it->update();
    }
}

void DevResourceLoader::shutdown()
{
    taskManager->verifyMainThread();
    for (ResourceManager* m : managers) {
        m->shutdown();
    }
    managers.clear();
    managersMap.clear();
    byName.clear();
}


bool DevResourceLoader::hasManagerForType(const std::string& resourceType) {
    return managersMap.find(resourceType) != managersMap.end();
}

DevResourceLoader::LoadedIndex* DevResourceLoader::findIndexByName(const std::string& string)
{
    for (u32 i = 0; i < resourceMetaIndexes.size(); ++i)
    {
        if (resourceMetaIndexes[i].name == string)
        {
            return &resourceMetaIndexes[i];
        }
    }
    return nullptr;
}

bool DevResourceLoader::loadIndex(const std::string& indexFilename)
{
    LoadedIndex* index = findIndexByName(indexFilename);

    std::ifstream file(indexFilename);
    if (file.is_open())
    {
        bool allowOverride = false;
        if (index == nullptr)
        {
            ptrsize indexId = resourceMetaIndexes.size();
            resourceMetaIndexes.push_back(LoadedIndex());
            index = &resourceMetaIndexes[indexId];
            index->metas.reserve(512);
            index->name = indexFilename;
            index->basefilepath = std::filesystem::path(indexFilename).parent_path().string();
        }
        else
        {
            allowOverride = true;
            index->metas.clear();
        }

        index->data = nlohmann::json::parse(file);
        file.close();

        loadPackages(index, allowOverride);

        // TODO: Make a proper arena
        u8 buffer[256];
        MemoryArena arena;

        if (allowOverride)
        {
            // Reload all resources in order of manager
            for (ResourceManager* m : managers)
            {
                for (DevResourceMeta& meta : index->metas)
                {
                    const ManagerInfo& info = managersMap[meta.type];
                    if (m == info.mngr)
                    {
                        // Check if the resource is loaded
                        // and the ask to realod it
                        if (meta.getReferences() > 0)
                        {
                            // Clear arena memory
                            arena.init(buffer, 256);

                            // If the resource have references, this means someone already requested it to be loaded.
                            DevPropHolder holder(this, meta.properties, arena);
                            BaseResource* resource = m->loadResource(&meta, &holder); // just get a reference
                            //m->reloadResource(resource);
                        }
                    }
                }
            }
        }
    }
    else
    {
        LOG(EngineLog, BE_LOG_ERROR) << "Failed to read file:\n" << indexFilename;
        return false;
    }

    // Show meta readed from file
    std::string log;
    for (ResourceMeta& meta : index->metas)
    {
        log += meta.toString();
    }
    LOG(EngineLog, BE_LOG_VERBOSE) << "Loaded Resource Metas:\n" << log;

    return true;
}




struct ByteSerializer {
    std::ofstream& ptr;

    template<typename T>
    void write(const char* name, T data) {
        ptr.write((const char*)&data, sizeof(T));
    }

    template<typename T>
    void write(const char* name, RR<T> data) {
        u32 id = data->getResourceId();
        this->write(name, id);
    }

};

struct SerializerTypeSize {
    template<typename T>
    void write(const char* name, T data) {
        size += sizeof(T);
    }

    u32 size = 0;
};

void DevResourceLoader::convertIndexesToProd() {
    using namespace BitEngine;

    std::vector<DevResourceMeta*> sprites;
    for (LoadedIndex& index : resourceMetaIndexes) {
        for (DevResourceMeta& meta : index.metas) {
            if (meta.type == "SPRITE") {
                sprites.emplace_back(&meta);
            }
        }
    }

    SerializerTypeSize spriteSerializerInfo;
    Sprite tmp;
    Sprite::serialize(&spriteSerializerInfo, &tmp);
    
    std::ofstream out("prod-index.data", std::ios::binary | std::ios::out);
    
    ByteSerializer byteSerializer { out };
    for (DevResourceMeta* meta : sprites) {
        RR<Sprite> sprite = getResource<Sprite>(meta->id);
        Sprite::serialize(&byteSerializer, sprite.get());
    }
    out.close();
}

DevResourceMeta* DevResourceLoader::findMeta(const std::string& name)
{
    const auto& it = byName.find(name);
    if (it == byName.end())
    {
        return nullptr;
    }
    else
    {
        return it->second;
    }
}

void DevResourceLoader::loadPackages(LoadedIndex* index, bool allowOverride)
{
    if (index->data["data"].empty())
    {
        return;
    }
    nlohmann::json::object_t& data = index->data["data"].get_ref<nlohmann::json::object_t&>();

#if BE_DEBUG
    std::set<std::string> typesWithoutManager;
#endif

    for (auto &package : data)
    {
        for (auto &resource : package.second)
        {
            const std::string& packageName = package.first;

            const std::string& resourceName = resource["name"];
            const std::string& resourceType = resource["type"];

            std::string name = getPackagePath(packageName, resourceName);
            
            auto found = byName.find(name);
            if (found != byName.end()) {
                found->second->properties = resource;
                std::string props = package.second.dump();
                printf(props.c_str());
            } else {
                index->metas.emplace_back(packageName, resourceName);
                DevResourceMeta& meta = index->metas.back();
                meta.type = resourceType;
                meta.properties = resource;
                if (resource.contains("filePath")) {
                    meta.filePath = index->basefilepath + "/" + resource["filePath"].get_ref<const std::string&>();
                }
                byName[name] = &meta; // Also index path
                byName[resourceName] = &meta;
            }

#if BE_DEBUG
            if (!isManagerForTypeAvailable(resourceType) && typesWithoutManager.find(resourceType) == typesWithoutManager.end()) {
                typesWithoutManager.emplace(resourceType);
                LOG(EngineLog, BE_LOG_WARNING) << "No resource manager for type " << resourceType;
            }
#endif
        }
    }
}

BaseResource* DevResourceLoader::loadResource(ResourceMeta* meta)
{
    DevResourceMeta* dmeta = static_cast<DevResourceMeta*>(meta);

    const ManagerInfo& info = managersMap[dmeta->type];
    
    // TODO: Make a proper arena
    u8 buffer[512];
    MemoryArena arena;
    arena.init(buffer, 512);

    DevPropHolder props(this, dmeta->properties, arena);
    BaseResource* resource = info.mngr->loadResource(dmeta, &props);

    return resource;
}

BaseResource* DevResourceLoader::loadResource(const u32 idx)
{
    DevResourceMeta* meta = byId[idx];
    if (meta == nullptr)
    {
        LOG(EngineLog, BE_LOG_ERROR) << "Couldn't find resource: '" << idx << "'";
        return nullptr;
    }
    else
    {
        return loadResource(meta);
    }
}

BaseResource* DevResourceLoader::loadResource(const std::string& meta) {
    const auto& found = byName.find(meta);
    if (found != byName.end()) {
        DevResourceMeta* meta = found->second;
        return loadResource(meta);
    }
    BE_INVALID_PATH("Name is not defined: " + meta); // Invalid code path
}

void DevResourceLoader::reloadResource(BaseResource* resource)
{
    // TODO: Force reload properties from file
    DevResourceMeta* meta = static_cast<DevResourceMeta*>(resource->getMeta());
    
    const ManagerInfo& info = managersMap[meta->type];

    info.mngr->reloadResource(resource);
}

void DevResourceLoader::releaseAll()
{
}

void DevResourceLoader::resourceNotInUse(ResourceMeta* meta)
{
    DevResourceMeta* dmeta = static_cast<DevResourceMeta*>(meta);
    managersMap[dmeta->type].mngr->resourceNotInUse(meta);
}

void DevResourceLoader::waitForAll()
{
}

void DevResourceLoader::waitForResource(BaseResource* resource)
{
}

bool DevResourceLoader::isManagerForTypeAvailable(const std::string& type)
{
    return managersMap.find(type) != managersMap.end();
}


}