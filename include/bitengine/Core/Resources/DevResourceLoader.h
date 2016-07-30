#pragma once

// Read json index file

#include "Core/Resources/ResourceManager.h"
#include "Core/Task.h"

#include "json.h"


namespace BitEngine 
{
	struct DevResourcePropertyRef : public ResourceProperty
	{
		DevResourcePropertyRef(nlohmann::json& json)
			: properties(json)
		{
		}
		~DevResourcePropertyRef() {
			LOG(EngineLog, BE_LOG_VERBOSE) << this << " was reference removed!";
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

	// Resource Loader implementation
	// This loader loads files from the standard file system
	class DevResourceLoader : public ResourceLoader
	{
		public:
			DevResourceLoader(GameEngine* ge);

			bool init() override;
			virtual void update() override;

			// Inherited via ResourceLoader
			virtual void shutdown() override;
			virtual void registerResourceManager(const std::string& resourceType, ResourceManager* manager) override;

			virtual bool loadIndex(const std::string& indexFilename) override;

			virtual ResourceMeta* findMeta(const std::string& name) override;

		protected:
			// Retrieve 
			virtual BaseResource* loadResource(const std::string& name) override;

			virtual void releaseAll() override;
			virtual void releaseResource(u32 id) override;

			virtual void waitForAll() override;
			virtual void waitForResource(BaseResource* resource) override;

			// Returns nullptr if meta conflicts
			ResourceMeta* addResourceMeta(const ResourceMeta& meta);


			// Returns the full directory where the file for this meta is.
			static std::string getDirectoryPath(const ResourceMeta* meta);
			static bool loadFileToMemory(const std::string& fname, std::vector<char>& out);

		private:

			class RawResourceLoaderTask : public Task
			{
				public:
				RawResourceLoaderTask(ResourceMeta* d, ResourceManager* out)
					: Task(TaskMode::NONE, Affinity::BACKGROUND), dr(d), putAt(out)
				{
				}

				// Inherited via Task
				void run() override;

				private:
					DataRequest dr;
					ResourceManager* putAt;
			};

			void requestResourceData(ResourceMeta* meta, ResourceManager* responseTo) override;
			
			bool isManagerForTypeAvailable(const std::string& type);
			BitEngine::BaseResource* getResourceFromManager(ResourceMeta* meta);

			void loadPackages(nlohmann::json::object_t& data);
			
			// Holds the managers
			std::unordered_map<std::string, ResourceManager*> managers;

			std::vector<ResourceMeta> resourceMeta;
			std::array<nlohmann::json, 8> resourceMetaIndexes;
			u32 loadedMetaIndexes;
			std::unordered_map<std::string, u32> byName;
	};

}