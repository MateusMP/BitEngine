#pragma once

// Read json index file

#include "Core/Resources/ResourceManager.h"


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
			DevResourceLoader();

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
			virtual void releaseResource(uint32 id) override;

			virtual void waitForAll() override;
			virtual void waitForResource(BaseResource* resource) override;

			// Returns nullptr if meta conflicts
			ResourceMeta* addResourceMeta(const ResourceMeta& meta);

			bool loadFileToMemory(const std::string& fname, std::vector<char>& out);

		private:
			struct LoadRequest {
				LoadRequest(LoadRequest&& lr)
					: dr(std::move(lr.dr)), putAt(lr.putAt)
				{}
				LoadRequest()
					: dr(nullptr)
				{}
				LoadRequest(ResourceMeta* d, ResourceManager* out)
					: dr(d), putAt(out)
				{}
				/*LoadRequest(const DataRequest& d, ResourceManager* out)
					: dr(d), putAt(out)
				{}*/
				LoadRequest& operator=(LoadRequest&& other) {
					dr = std::move(other.dr);
					putAt = other.putAt;
					return *this;
				}
				/*LoadRequest& operator=(const LoadRequest& other) {
					dr = other.dr;
					putAt = other.putAt;
					return *this;
				}*/

				DataRequest dr;
				ResourceManager* putAt;
			};

			void requestResourceData(ResourceMeta* meta, ResourceManager* responseTo) override;
			
			bool isManagerForTypeAvailable(const std::string& type);
			BitEngine::BaseResource* getResourceFromManager(ResourceMeta* meta);

			void loadPackages(nlohmann::json::object_t& data);

			// Executed on the loader thread
			void dataLoaderLoop();

			// Returns the full directory where the file for this meta is.
			std::string getDirectoryPath(const ResourceMeta* meta);

			// Holds the managers
			std::unordered_map<std::string, ResourceManager*> managers;

			std::thread loaderThread;
			bool working;
			std::vector<ResourceMeta> resourceMeta;
			std::array<nlohmann::json, 8> resourceMetaIndexes;
			uint32 loadedMetaIndexes;
			std::unordered_map<std::string, uint32> byName;

			// Load requests
			ThreadSafeQueue<LoadRequest> loadRequests;
	};

}