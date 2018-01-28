#pragma once

// Read json index file

#include "bitengine/Core/Resources/ResourceManager.h"
#include "bitengine/Core/Task.h"
#include "bitengine/Core/Memory.h"

#include "json.hpp"


namespace BitEngine 
{
	struct DevResourceMeta : public ResourceMeta {
		std::string filePath; // In case the resource is associated with a file

		DevResourceMeta(const std::string& pack)
			: ResourceMeta(pack)
		{}
	};

	struct DevResourcePropertyRef : public ResourceProperty
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
	class DevResourceLoader : public ResourceLoader
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


			DevResourceLoader(MemoryArena& memoryArena,  Messenger* ge, TaskManager* taskManager);
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
			std::vector<ResourceManager*> managers;
			std::unordered_map<std::string, ResourceManager*> managersMap;

			std::vector<DevResourceMeta> resourceMeta;
			std::vector<LoadedIndex> resourceMetaIndexes;
			std::unordered_map<std::string, u32> byName;

			TaskManager* taskManager;

			MemoryArena& memoryArena;
	};

}
