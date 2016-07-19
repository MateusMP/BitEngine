#include <fstream>

#include "Core/Logger.h"
#include "Core/Resources/DevResourceLoader.h"

#include "json.h"

const std::string BitEngine::ResourceMeta::toString() const {
	return ("Resource Meta id: " + std::to_string(id) +
		"\n\tpackage: " + package +
		"\n\ttype: " + type +
		"\n\tfiledDir: " + resourceName +
		"\n\tprops: " + properties.dump() + "\n");

}

BitEngine::DevResourceLoader::DevResourceLoader()
	: working(true)
{}

bool BitEngine::DevResourceLoader::init()
{
	loaderThread = std::thread(&DevResourceLoader::dataLoaderLoop, this);

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
	working = false;
	loaderThread.join();
}


void BitEngine::DevResourceLoader::registerResourceManager(const std::string & resourceType, ResourceManager * manager)
{
	managers[resourceType] = manager;
}

void BitEngine::DevResourceLoader::loadIndex(const std::string& indexFilename)
{
	LOG(EngineLog, BE_LOG_VERBOSE) << "Loading resource index " << indexFilename;
	std::ifstream file(indexFilename, std::ios::in | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	LOG(EngineLog, BE_LOG_VERBOSE) << indexFilename << " size: " << size;

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	if ( file.eof() )
	{
		nlohmann::json j = nlohmann::json::parse(buffer.data());
		
		if (!j["data"].empty())
		{
			loadPackages(j["data"].get<nlohmann::json::object_t>());
		}
	}
	else
	{
		LOG(EngineLog, BE_LOG_ERROR) << "Failed to read file:\n" << buffer.data();
	}

	// Show meta readed from file
	std::string log;
	for (ResourceMeta& meta : resourceMeta)
	{
		log += meta.toString();
	}
	LOG(EngineLog, BE_LOG_VERBOSE) << "Loaded Resource Metas:\n" << log;
}

void BitEngine::DevResourceLoader::loadPackages(nlohmann::json::object_t& data)
{
	for (auto &package : data)
	{
		for (auto &resource : package.second)
		{
			BitEngine::ResourceMeta tmpResMeta(package.first);

			for (auto &property : resource.get<nlohmann::json::object_t>())
			{
				if (property.first == "name")
				{
					tmpResMeta.resourceName = property.second.get<std::string>();
				}
				else if (property.first == "type")
				{
					const std::string& type = property.second.get<std::string>();
					tmpResMeta.type = type;
					if (!isManagerForTypeAvailable(type)) {
						LOG(EngineLog, BE_LOG_WARNING) << "No resource manager for type " << type;
					}
				}
				else
				{
					tmpResMeta.properties[property.first] = property.second;
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
	auto it = byName.find(name);
	if (it == byName.end())
	{
		// create new entry
		LOG(EngineLog, BE_LOG_ERROR) << "Couldn't find resource: '" << name << "'";
		return nullptr;
	}
	else
	{
		return getResourceFromManager(it->second);
	}
}

void BitEngine::DevResourceLoader::releaseAll()
{
}

void BitEngine::DevResourceLoader::releaseResource(uint32 id)
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
		uint32 id = resourceMeta.size();
		resourceMeta.emplace_back(meta);

		ResourceMeta& newRm = resourceMeta.back();
		newRm.id = id;
		byName[fullPath] = &newRm;
		return &newRm;
	}
	else
	{
		return nullptr;
	}
}

void BitEngine::DevResourceLoader::requestResourceData(ResourceMeta* meta, ThreadSafeQueue<DataRequest>* responseTo)
{
	loadRequests.push(LoadRequest(meta, responseTo));
}

bool BitEngine::DevResourceLoader::isManagerForTypeAvailable(const std::string& type)
{
	return managers.find(type) != managers.end();
}

BitEngine::BaseResource* BitEngine::DevResourceLoader::getResourceFromManager(ResourceMeta* meta)
{
	return managers[meta->type]->loadResource(meta);
}

void BitEngine::DevResourceLoader::dataLoaderLoop()
{
	while (working)
	{
		LoadRequest lr;
		if (loadRequests.tryPop(lr))
		{
			lr.dr.loadState = DataRequest::LoadState::LOADING;
			const std::string path = getDirectoryPath(lr.dr.meta);

			LOG(EngineLog, BE_LOG_VERBOSE) << "Data Loader: " << path;

			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				long fSize = file.tellg();
				lr.dr.data.resize(fSize);
				file.read(lr.dr.data.data(), fSize);
				if (file.eof())
				{
					lr.dr.loadState = DataRequest::LoadState::LOADED;
					lr.putAt->push(lr.dr);
				}
			}
			else
			{
				LOG(EngineLog, BE_LOG_ERROR) << "Failed to open file: " << path;
				lr.dr.loadState = DataRequest::LoadState::ERROR;
			}
		}
		else
		{
			std::this_thread::yield();
		}
	}
}

std::string BitEngine::DevResourceLoader::getDirectoryPath(const ResourceMeta* meta)
{
	return "data/" + meta->package + "/" + meta->resourceName;
}