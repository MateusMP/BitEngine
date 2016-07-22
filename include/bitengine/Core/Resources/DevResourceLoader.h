#pragma once

// Read json index file

#include "Core/Resources/ResourceManager.h"

namespace BitEngine 
{

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
			virtual void registerResourceManager(const std::string & resourceType, ResourceManager * manager) override;

			virtual void loadIndex(const std::string& indexFilename) override;

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


		private:
			struct LoadRequest {
				LoadRequest(LoadRequest&& lr)
					: dr(std::move(lr.dr)), putAt(lr.putAt)
				{}
				LoadRequest()
					: dr(nullptr)
				{}
				LoadRequest(const DataRequest& d, ResourceManager* out)
					: dr(d), putAt(out)
				{}
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
			std::unordered_map<std::string, uint32> byName;

			// Load requests
			ThreadSafeQueue<LoadRequest> loadRequests;
	};

}