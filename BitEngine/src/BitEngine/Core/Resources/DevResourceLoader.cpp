#include <fstream>

#include "BitEngine/Core/Resources/DevResourceLoader.h"

#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/Task.h"
#include "BitEngine/Core/TaskManager.h"

namespace BitEngine {

// Load task
class DevLoaderTask : public ResourceLoader::RawResourceLoaderTask
{
public:
    DevLoaderTask(DevResourceLoader* loader, ResourceMeta* meta)
        : RawResourceLoaderTask(meta), loader(loader)
    {}

    void DevLoaderTask::run() override
    {
        using namespace BitEngine;

        dr.loadState = ResourceLoader::DataRequest::LoadState::LS_LOADING;
        DevResourceMeta* drm = static_cast<DevResourceMeta*>(dr.meta);
        const std::string& path = drm->filePath;

        LOG(EngineLog, BE_LOG_VERBOSE) << "Data Loader: " << path;

        if (path.empty()) {
            throw "EMPTY PATH FOR RESOURCE";
        }

        if (DevResourceLoader::loadFileToMemory(path, dr.data))
        {
            dr.loadState = ResourceLoader::DataRequest::LoadState::LS_LOADED;
            loader->finishedLoading(dr.meta);
        }
        else
        {
            LOG(EngineLog, BE_LOG_ERROR) << "Failed to open file: " << path;
            dr.loadState = ResourceLoader::DataRequest::LoadState::LS_ERROR;
        }
    }

private:
    DevResourceLoader* loader;
};


//

DevResourceLoader::DevResourceLoader(MemoryArena& memory, TaskManager* tm)
    : memoryArena(memory), taskManager(tm)
{
    resourceMeta.reserve(4096);
    resourceMetaIndexes.reserve(8);
}

DevResourceLoader::~DevResourceLoader()
{

}

bool DevResourceLoader::init()
{
    taskManager->verifyMainThread();
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

void DevResourceLoader::registerResourceManager(const std::string& resourceType, ResourceManager* manager)
{
    taskManager->verifyMainThread();
    BE_ASSERT(manager != nullptr);
    manager->setResourceLoader(this);
    managers.emplace_back(manager);
    managersMap[resourceType] = manager;
}

bool DevResourceLoader::hasManagerForType(const std::string& resourceType) {
    return managersMap.find(resourceType) != managersMap.end();
}

ResourceMeta* DevResourceLoader::includeMeta(const std::string& package, const std::string& resourceName,
    const std::string& type, ResourcePropertyContainer properties)
{
    DevResourceMeta meta(package);
    meta.resourceName = resourceName;
    meta.type = type;
    meta.properties = properties;
    meta.filePath = "<in-memory>";
    return addResourceMeta(meta, false);
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
            u32 indexId = resourceMetaIndexes.size();
            resourceMetaIndexes.push_back(LoadedIndex());
            index = &resourceMetaIndexes[indexId];
            index->name = indexFilename;
        }
        else
        {
            allowOverride = true;
            index->metas.clear();
        }

        index->data = nlohmann::json::parse(file);
        loadPackages(index, allowOverride);

        if (allowOverride)
        {
            // Reload all resources in order of manager
            for (ResourceManager* m : managers)
            {
                for (DevResourceMeta* meta : index->metas)
                {
                    if (m == managersMap[meta->type])
                    {
                        // Check if the resource is loaded
                        // and the ask to realod it
                        if (meta->getReferences() > 0)
                        {
                            // If the resource have references, this means someone already requested it to be loaded.
                            BaseResource* r = m->loadResource(meta); // just get a reference
                            m->reloadResource(r);
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
    for (ResourceMeta& meta : resourceMeta)
    {
        log += meta.toString();
    }
    LOG(EngineLog, BE_LOG_VERBOSE) << "Loaded Resource Metas:\n" << log;

    return true;
}

ResourceMeta* DevResourceLoader::findMeta(const std::string& name)
{
    const auto& it = byName.find(name);
    if (it == byName.end())
    {
        return nullptr;
    }
    else
    {
        return &resourceMeta[it->second];
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
            DevResourceMeta tmpResMeta(package.first);

            for (auto &property : resource.get_ref<nlohmann::json::object_t&>())
            {
                if (property.first == "name")
                {
                    tmpResMeta.resourceName = property.second.get_ref<std::string&>();
                }
                else if (property.first == "type")
                {
                    const std::string& type = property.second.get_ref<std::string&>();
                    tmpResMeta.type = type;
#if BE_DEBUG
                    if (!isManagerForTypeAvailable(type) && typesWithoutManager.find(type) == typesWithoutManager.end()) {
                        typesWithoutManager.emplace(type);
                        LOG(EngineLog, BE_LOG_WARNING) << "No resource manager for type " << type;
                    }
#endif
                }
                else if (property.first == "filePath")
                {
                    tmpResMeta.filePath = property.second.get<std::string>();
                }
                else
                {
                    DevResourcePropertyRef* ref = memoryArena.push<DevResourcePropertyRef>(resource);
                    tmpResMeta.properties = ResourcePropertyContainer(ref);
                }
            }

            DevResourceMeta* meta = addResourceMeta(tmpResMeta, allowOverride);
            if (meta == nullptr)
            {
                LOG(EngineLog, BE_LOG_ERROR) << "Meta would override another one.\n" << tmpResMeta.toString();
            }
            else
            {
                index->metas.emplace_back(meta);
            }
        }
    }
}

BaseResource* DevResourceLoader::loadResource(const std::string& name)
{
    ResourceMeta* meta = findMeta(name);
    if (meta == nullptr)
    {
        LOG(EngineLog, BE_LOG_ERROR) << "Couldn't find resource: '" << name << "'";
        return nullptr;
    }
    else
    {
        return getResourceFromManager(meta);
    }
}

void DevResourceLoader::reloadResource(BaseResource* resource)
{
    ResourceMeta* meta = resource->getMeta();
    managersMap[meta->type]->reloadResource(resource);
}

void DevResourceLoader::releaseAll()
{
}

void DevResourceLoader::resourceNotInUse(ResourceMeta* meta)
{
    managersMap[meta->type]->resourceNotInUse(meta);
}

void DevResourceLoader::waitForAll()
{
}

void DevResourceLoader::waitForResource(BaseResource* resource)
{
}

DevResourceMeta* DevResourceLoader::addResourceMeta(const DevResourceMeta &meta, bool allowOverride)
{
    taskManager->verifyMainThread();
    const std::string fullPath = getPackagePath(&meta);
    auto it = byName.find(fullPath);
    if (it == byName.end())
    {
        u32 id = resourceMeta.size();
        resourceMeta.emplace_back(meta);

        DevResourceMeta& newRm = resourceMeta.back();
        newRm.id = id;
        byName[fullPath] = id;
        newRm.properties = meta.properties;
        return &resourceMeta[id];
    }
    else if (allowOverride) // Override current loaded options
    {
        DevResourceMeta& rm = resourceMeta[it->second];
        rm.properties = meta.properties;
        rm.type = meta.type;
        return &rm;
    }
    else
    {
        return nullptr;
    }
}

ResourceLoader::RawResourceTask DevResourceLoader::requestResourceData(ResourceMeta* meta)
{
    RawResourceTask task = std::make_shared<DevLoaderTask>(this, meta);
    taskManager->addTask(task);
    return task;
}

bool DevResourceLoader::isManagerForTypeAvailable(const std::string& type)
{
    return managersMap.find(type) != managersMap.end();
}

BaseResource* DevResourceLoader::getResourceFromManager(ResourceMeta* meta)
{
    return managersMap[meta->type]->loadResource(meta);
}

// Static

std::string DevResourceLoader::getPackagePath(const ResourceMeta* meta)
{
    return "data/" + meta->package + "/" + meta->resourceName;
}

bool DevResourceLoader::loadFileToMemory(const std::string& fname, std::vector<char>& out)
{
    LOG(EngineLog, BE_LOG_VERBOSE) << "Loading resource index " << fname;
    std::ifstream file(fname, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return false;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(size));

    LOG(EngineLog, BE_LOG_VERBOSE) << fname << " size: " << size;

    file.read(out.data(), size);
    return file.gcount() == size;
}

}