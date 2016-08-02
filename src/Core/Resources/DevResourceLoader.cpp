#include <fstream>

#include "Core/Resources/DevResourceLoader.h"

#include "Core/Logger.h"
#include "Core/Task.h"
#include "Core/TaskManager.h"

#include "json.h"


// Load task
class DevLoaderTask : public BitEngine::ResourceLoader::RawResourceLoaderTask
{
	public:
	DevLoaderTask(BitEngine::ResourceMeta* meta) 
		: RawResourceLoaderTask(meta)
	{}

	// Inherited via RawResourceLoaderTask
	void run() override;

};

const std::string BitEngine::ResourceMeta::toString() const {
	return ("Resource Meta id: " + std::to_string(id) +
		"\n\tpackage: " + package +
		"\n\ttype: " + type +
		"\n\tfiledDir: " + resourceName +
		"\n\tprops: " + /*properties.dump() + */"\n");
}

BitEngine::DevResourceLoader::DevResourceLoader(GameEngine* ge)
	: ResourceLoader(ge), loadedMetaIndexes(0)
{
	resourceMeta.reserve(4096);
}

bool BitEngine::DevResourceLoader::init()
{
	for (auto& it : managers)
	{
		it.second->init();
	}
	return true;
}

void BitEngine::DevResourceLoader::update()
{
	for (auto& it : managers)
	{
		it.second->update();
	}
}

void BitEngine::DevResourceLoader::shutdown()
{

}


void BitEngine::DevResourceLoader::registerResourceManager(const std::string & resourceType, ResourceManager * manager)
{
	if (manager == nullptr)
	{
		LOG(EngineLog, BE_LOG_ERROR) << "Registering invalid manager for type " << resourceType;
	}
	else
	{
		manager->setResourceLoader(this);
	}
	managers[resourceType] = manager;
}

bool BitEngine::DevResourceLoader::loadFileToMemory(const std::string& fname, std::vector<char>& out)
{
	LOG(EngineLog, BE_LOG_VERBOSE) << "Loading resource index " << fname;
	std::ifstream file(fname, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		LOG(EngineLog, BE_LOG_ERROR) << "Failed to open index file " << fname;
		return false;
	}
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	out.resize(static_cast<size_t>(size));

	LOG(EngineLog, BE_LOG_VERBOSE) << fname << " size: " << size;

	file.read(out.data(), size);
	return file.gcount() == size;
}

BitEngine::ResourceMeta* BitEngine::DevResourceLoader::includeMeta(const std::string& package, const std::string& resourceName,
	const std::string& type, ResourcePropertyContainer properties)
{
	ResourceMeta meta(package);
	meta.resourceName = resourceName;
	meta.type = type;
	meta.properties = properties;
	return addResourceMeta(meta);
}

bool BitEngine::DevResourceLoader::loadIndex(const std::string& indexFilename)
{
	u32 indexId = loadedMetaIndexes;
	loadedMetaIndexes++;

	std::ifstream file(indexFilename);
	if (file.is_open())
	{
		// TODO: Save this to use only referenced ResourceProperty
		resourceMetaIndexes[indexId] = nlohmann::json(file);
		nlohmann::json& j = resourceMetaIndexes[indexId];
		
		if (!j["data"].empty())
		{
			loadPackages(j["data"].get_ref<nlohmann::json::object_t&>());
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

BitEngine::ResourceMeta* BitEngine::DevResourceLoader::findMeta(const std::string& name)
{
	auto& it = byName.find(name);
	if (it == byName.end())
	{
		return nullptr;
	}
	else
	{
		return &resourceMeta[it->second];
	}
}

void BitEngine::DevResourceLoader::loadPackages(nlohmann::json::object_t& data)
{
	for (auto &package : data)
	{
		for (auto &resource : package.second)
		{
			BitEngine::ResourceMeta tmpResMeta(package.first);

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
					if (!isManagerForTypeAvailable(type)) {
						LOG(EngineLog, BE_LOG_WARNING) << "No resource manager for type " << type;
					}
				}
				else
				{
					tmpResMeta.properties = ResourcePropertyContainer(new DevResourcePropertyRef(resource));
				}
			}

			if (addResourceMeta(tmpResMeta) == nullptr)
			{
				LOG(EngineLog, BE_LOG_ERROR) << "Meta would override another one.\n" << tmpResMeta.toString();
			}
		}
	}
}

BitEngine::BaseResource* BitEngine::DevResourceLoader::loadResource(const std::string& name)
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

void BitEngine::DevResourceLoader::releaseAll()
{
}

void BitEngine::DevResourceLoader::releaseResource(u32 id)
{
}

void BitEngine::DevResourceLoader::waitForAll()
{
}

void BitEngine::DevResourceLoader::waitForResource(BaseResource* resource)
{
}

BitEngine::ResourceMeta* BitEngine::DevResourceLoader::addResourceMeta(const ResourceMeta& meta)
{
	const std::string fullPath = getDirectoryPath(&meta);
	auto it = byName.find(fullPath);
	if (it == byName.end())
	{
		u32 id = resourceMeta.size();
		resourceMeta.emplace_back(meta);

		ResourceMeta& newRm = resourceMeta.back();
		newRm.id = id;
		byName[fullPath] = id;
		newRm.properties = meta.properties;
		return &resourceMeta[id];
	}
	else
	{
		return nullptr;
	}
}

BitEngine::ResourceLoader::RawResourceTask BitEngine::DevResourceLoader::requestResourceData(ResourceMeta* meta)
{
	RawResourceTask task = std::make_shared<DevLoaderTask>(meta);
	getEngine()->getTaskManager()->addTask(task);
	return task;
}

bool BitEngine::DevResourceLoader::isManagerForTypeAvailable(const std::string& type)
{
	return managers.find(type) != managers.end();
}

BitEngine::BaseResource* BitEngine::DevResourceLoader::getResourceFromManager(ResourceMeta* meta)
{
	return managers[meta->type]->loadResource(meta);
}

std::string BitEngine::DevResourceLoader::getDirectoryPath(const ResourceMeta* meta)
{
	return "data/" + meta->package + "/" + meta->resourceName;
}



void DevLoaderTask::run()
{
	using namespace BitEngine;

	dr.loadState = ResourceLoader::DataRequest::LoadState::LOADING;
	const std::string path = DevResourceLoader::getDirectoryPath(dr.meta);

	LOG(EngineLog, BE_LOG_VERBOSE) << "Data Loader: " << path;

	if (DevResourceLoader::loadFileToMemory(path, dr.data))
	{
		dr.loadState = ResourceLoader::DataRequest::LoadState::LOADED;
	}
	else
	{
		LOG(EngineLog, BE_LOG_ERROR) << "Failed to open file: " << path;
		dr.loadState = ResourceLoader::DataRequest::LoadState::ERROR;
	}
}