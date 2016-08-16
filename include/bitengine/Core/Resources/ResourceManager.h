#pragma once

#include <string>
#include <vector>
#include <map>

#include "Common/TypeDefinition.h"
#include "Common/ThreadSafeQueue.h"
#include "Core/Resources/ResourceIndexer.h"
#include "Core/Logger.h"
#include "Core/GameEngine.h"
#include "Core/Task.h"

namespace BitEngine {

	class ResourceManager;

	struct ResourceProperty;
	struct ResourcePropertyContainer;

	struct ResourceProperty
	{
		virtual ~ResourceProperty(){}
		virtual ResourcePropertyContainer getProperty(const std::string& str) = 0;
		virtual ResourcePropertyContainer getProperty(int index) = 0;
		virtual const std::string& getValueString() const = 0;
		virtual double getValueDouble() const = 0;
		virtual int getValueInt() const = 0;
		virtual int getNumberOfProperties() const = 0;
		//virtual int getKeys(std::vector<std::string>& keys) = 0;

		virtual ResourcePropertyContainer operator[](const std::string& str) = 0; // { return this->getProperty(str); }
	};

	struct ResourcePropertyContainer
	{
		ResourcePropertyContainer()
		{}
		ResourcePropertyContainer(ResourceProperty* p) 
			: prop(p)
		{}

		
		ResourcePropertyContainer operator[](const std::string& str) {
			return prop->getProperty(str);
		}
		const ResourcePropertyContainer operator[](const std::string& str) const {
			return prop->getProperty(str);
		}
		ResourcePropertyContainer operator[](int index) {
			return prop->getProperty(index);
		}
		const ResourcePropertyContainer operator[](int index) const {
			return prop->getProperty(index);
		}
		std::string getValueString() const { return prop->getValueString(); }
		double getValueDouble() const { return prop->getValueDouble(); }
		int getValueInt() const { return prop->getValueInt(); }
		int getNumberOfProperties() const { return prop->getNumberOfProperties(); }

		bool isValid() { return prop.get() != nullptr; }

	private:
		std::shared_ptr<ResourceProperty> prop;
	};

	struct ResourceMeta
	{
		ResourceMeta(const std::string& pack)
			: id(~0), package(pack)
		{}

		const std::string toString() const;

		u32 id;
		std::string package;
		std::string resourceName;
		std::string type;
		ResourcePropertyContainer properties;
	};

	// All resources types should come from this
	class BaseResource
	{
		public:
			BaseResource()
				: meta(nullptr)
			{}

			// Base resource
			// d Owns the data from this vector
			BaseResource(ResourceMeta* _meta)
				: meta(_meta)
			{}
			
			u32 getResourceId() const {
				return meta->id;
			}

			ResourceMeta* getMeta() {
				return meta;
			}

		protected:
			ResourceMeta* meta;
	};
	
	// Resource Loader interface
	// Used by the application to retrieve the final resource.
	class ResourceLoader : public EnginePiece
	{
		public:
			ResourceLoader(GameEngine* ge) : EnginePiece(ge) {}
		    virtual ~ResourceLoader(){}

			virtual bool init() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void registerResourceManager(const std::string& resourceType, ResourceManager* manager) = 0;

			// Blocking call
			// Read a file and map all indices
			// If any resource override would happen, they're skipped
			// and a warning is logged.
			// \return true if succeded
			virtual bool loadIndex(const std::string& index) = 0;

			virtual BitEngine::ResourceMeta* includeMeta(const std::string& package, const std::string& resourceName,
									 const std::string& type, ResourcePropertyContainer properties = ResourcePropertyContainer()) = 0;
			
			// Find a resource meta for a given name
			virtual ResourceMeta* findMeta(const std::string& name) = 0;

			virtual void releaseAll() = 0;
			virtual void releaseResource(u32 id) = 0;

			template<typename T>
			T* getResource(const std::string& name) {
				return static_cast<T*>(loadResource(name));
			}


			// Returns the request ID
			// Queue the request to be loaded by another thread
			//virtual u32 loadRequest(const std::string& path, BaseResource* into, ResourceManager* callback) = 0;
			//virtual bool reloadResource(u32 resourceID, ResourceManager* callback) = 0;

			virtual void waitForAll() = 0;
			virtual void waitForResource(BaseResource* resource) = 0;


			struct DataRequest
			{
				enum LoadState {
					NOT_LOADED,
					LOADING,
					LOADED,
					ERROR,
				};
				DataRequest(DataRequest&& dr) noexcept
					: meta(dr.meta), loadState(dr.loadState), data(std::move(dr.data))
				{}

				DataRequest(ResourceMeta* _meta)
					: meta(_meta), loadState(NOT_LOADED)
				{}
				DataRequest& operator=(DataRequest&& other) {
					meta = other.meta;
					loadState = other.loadState;
					data = std::move(other.data);
					return *this;
				}

				bool isLoaded() {
					return loadState == LOADED;
				}

				/*DataRequest& operator=(const DataRequest& other) {
				meta = other.meta;
				loadState = other.loadState;
				data = other.data;
				return *this;
				}*/

				ResourceMeta* meta;
				LoadState loadState;
				std::vector<char> data;
			};

			// Load file data from file system
			class RawResourceLoaderTask : public Task
			{
				public:
				RawResourceLoaderTask(ResourceMeta* d)
					: Task(TaskMode::NONE, Affinity::BACKGROUND), dr(d)
				{
				}

				// Inherited via Task
				virtual void run() = 0;

				DataRequest& getData() {
					return dr;
				}

				protected:
					DataRequest dr;
			};

			typedef std::shared_ptr<RawResourceLoaderTask> RawResourceTask;
		protected:
			friend class ResourceManager;

			// Non blocking call
			// Retrieve the required resource by name
			// The resource is loaded on the first request
			// and stay loaded until not required anymore by any instance****
			// A temporary resource may be loaded, like a temporary texture or null sound.
			// To guarantee that the resource returned by this call is ready to use
			// follow this call by a waitForAll() or waitForResource(name)
			virtual BaseResource* loadResource(const std::string& name) = 0;

			// Used internally by resource managers, to retrieve raw resource data
			// Like, texture image data, sound data, and others.
			// This will create a background task to load the resource.
			// The task shall be sent to the TaskManager before this function returns.
			virtual RawResourceTask requestResourceData(ResourceMeta* meta) = 0;
	};

	// Resource manager interface
	// Used to load and handle each resource type
	class ResourceManager
	{
		public:
			virtual bool init() = 0;

			virtual void update() = 0;

			virtual void setResourceLoader(ResourceLoader* loader) = 0;

			virtual BaseResource* loadResource(ResourceMeta* meta) = 0;
			
			// in bytes
			virtual u32 getCurrentRamUsage() const = 0;
			virtual u32 getCurrentGPUMemoryUsage() const = 0;

			const std::map<ResourceMeta*, ResourceLoader::RawResourceTask>& getPendingToLoad() const {
				return waitingData;
			}

		protected:
			ResourceLoader::RawResourceTask loadRawData(ResourceLoader* loader, ResourceMeta* meta) {
				auto it = waitingData.emplace(meta, nullptr);
				if (it.second) {
					it.first->second = loader->requestResourceData(meta);
				}
				return it.first->second;
			}

			void finishedLoading(ResourceMeta* meta) {
				waitingData.erase(meta);
			}

		private:
			std::map<ResourceMeta*, ResourceLoader::RawResourceTask> waitingData; // the resources that are waiting the raw data to be loaded
	};

}
