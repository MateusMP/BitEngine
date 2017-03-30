#pragma once

#include <string>
#include <vector>
#include <map>
#include <type_traits>

#include "bitengine/Common/TypeDefinition.h"
#include "bitengine/Common/ThreadSafeQueue.h"
#include "bitengine/Core/Logger.h"
#include "bitengine/Core/GameEngine.h"
#include "bitengine/Core/Task.h"
#include "bitengine/Core/Resources/ResourceIndexer.h"
#include "bitengine/Core/Resources/ResourceProperties.h"

namespace BitEngine {

	template<typename T>
	class RR;

	/***
	 * Stores resource meta data.
	 */
	struct ResourceMeta
	{
		ResourceMeta(const std::string& pack)
			: id(~0), package(pack), references(0)
		{}

		const std::string toString() const;

		u32 id; /// unique resource id
		std::string package;
		std::string resourceName;
		std::string type;
		ResourcePropertyContainer properties;

		u32 getReferences() const {
			return references;
		}

		std::string getNameId() const {
			return "[" + type + "]" + package + "/" + resourceName;
		}

	private:
		friend class ResourceLoader;
		u32 references;
	};

	/**
	 * All resources types should come from this
	 */
	class BaseResource
	{
		public:
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
	
	/**
	 * Resource Loader interface
	 * Used by the application to retrieve the final resource.
	 */
	class ResourceLoader : public EnginePiece
	{
		public:
			ResourceLoader(GameEngine* ge) : EnginePiece(ge) {}
		    virtual ~ResourceLoader(){}

			virtual bool init() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			/**
			 * Register a resource manage to take case of a given resource type.
			 * @param resourceType the resource type.
			 * @param manager the manager
			 */
			virtual void registerResourceManager(const std::string& resourceType, ResourceManager* manager) = 0;

			/**
			 * TODO: This should not exist here. Loading from a file is not in this scope.
			 * Blocking call
			 * Read a file and map all indices
			 * If any resource override would happen, they're skipped
			 * and a warning is logged.
			 * @param index The index file to load
			 * @return true if succeded
			 */
			virtual bool loadIndex(const std::string& index) = 0;

			/**
			 * Include a meta in the internal index.
			 * @param package
			 * @param resourceName
			 * @param type
			 * @param properties
			 * @return
			 */
			virtual BitEngine::ResourceMeta* includeMeta(const std::string& package, const std::string& resourceName,
									 const std::string& type, ResourcePropertyContainer properties = ResourcePropertyContainer()) = 0;
			
			/**
			 * Find a resource meta for a given name.
			 * @param name the name to search for.
			 * @return the resource meta
			 */
			virtual ResourceMeta* findMeta(const std::string& name) = 0;

			/**
			 * Retrieve resource by name.
			 * @param name the resource name.
			 * @return a reference to the resource. It might not be fully loaded yet!
			 */
			template<typename T>
			RR<T> getResource(const std::string& name) {
				T* resource = static_cast<T*>(loadResource(name));
				return RR<T>(resource, this);
			}

			/**
			 * Force a resource to be reloaded.
			 * @param resource the resource reference
			 * @param wait if waiting the resource to be fully loaded is required, pass true here.
			 */
			template<typename T>
			void reloadResource(RR<T>& resource, bool wait = false) {
				reloadResource(resource.get());
				if (wait){
					waitForResource(resource.get());
				}
			}
			
			// Returns the request ID
			// Queue the request to be loaded by another thread
			//virtual u32 loadRequest(const std::string& path, BaseResource* into, ResourceManager* callback) = 0;
			//virtual bool reloadResource(u32 resourceID, ResourceManager* callback) = 0;

			/**
			 * Wait all resources to be loaded.
			 * This only considers resources that were previously requested.
			 */
			virtual void waitForAll() = 0;

			/**
			 * Wait a specifc resource to be loaded.
			 * @param resource the resouce.
			 */
			virtual void waitForResource(BaseResource* resource) = 0;

			/**
			 * Release all resources in use. TAKE CARE WITH THIS!
			 */
			virtual void releaseAll() = 0;

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

			/**
			 * This task is used when we need to load
			 * file data from the file system.
			 */
			class RawResourceLoaderTask : public Task
			{
				public:
					RawResourceLoaderTask(ResourceMeta* d)
						: Task(TaskMode::NONE, Affinity::BACKGROUND), dr(d)
					{}

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
			template<typename T> friend class RR;

			void incReference(BaseResource* r) {
				++(r->getMeta()->references);
			}

			void decReference(BaseResource* r) {
				ResourceMeta* meta = r->getMeta();
				if (--(meta->references) == 0){
					resourceNotInUse(meta); // TODO: Delegate this to be done by a task? Or later?
				}
			}

			/**
			 * Called when the given Resource Meta is not in use anymore
			 * This will only be called after a previous call to loadResource() was made
			 * Release the resource or not is up to the implementation
			 * Usually cheap resources are kept in memory (may be released from drivers)
			 */
			virtual void resourceNotInUse(ResourceMeta* meta) = 0;

			/** Non blocking call
			 * Retrieve the required resource by name
			 * The resource is loaded on the first request
			 * and stay loaded until not required anymore by any instance****
			 * A temporary resource may be loaded, like a temporary texture or null sound.
			 * To guarantee that the resource returned by this call is ready to use
			 * follow this call by a waitForAll() or waitForResource(name)
			 * @param name
			 * @return
			 */
			virtual BaseResource* loadResource(const std::string& name) = 0;

			// Will force a resource to be reloaded.
			virtual void reloadResource(BaseResource* resource) = 0;

			// Used internally by resource managers, to retrieve raw resource data
			// Like, texture image data, sound data, and others.
			// This will create a background task to load the resource.
			// The task shall be sent to the TaskManager before this function returns.
			virtual RawResourceTask requestResourceData(ResourceMeta* meta) = 0;
	};

	/**
	 * Resource Reference class.
	 * This is a Reference Counted reference to a given resource.
	 */
	template<typename T>
	class RR
	{
		friend class ResouceLoader;
		static_assert(std::is_base_of<BaseResource, T>::value, "Not a resource class");

	public:
		RR()
			: resource(nullptr), loader(nullptr)
		{}
		RR(T* r, ResourceLoader* l)
			: resource(r), loader(l)
		{
			incRef();
		}
		RR(RR&& r)
			: resource(r.resource), loader(r.loader)
		{}
		RR(const RR& r)
			: resource(r.resource), loader(r.loader)
		{
			incRef();
		}
		~RR()
		{
			decRef();
		}
		RR& operator=(RR&& r)
		{
			resource = r.resource;
			loader = r.loader;
			return *this;
		}
		RR& operator=(const RR& r)
		{
			resource = r.resource;
			loader = r.loader;
			incRef();
			return *this;
		}
		T* operator->() {
			return resource;
		}
		const T* operator->() const {
			return resource;
		}

		bool operator !=(const RR& r) const
		{
			return resource == r.resource && loader == r.loader;
		}

		bool isValid() const {
			return resource != nullptr && loader != nullptr;
		}

		T* get() {
			return resource;
		}

		const T* get() const {
			return resource;
		}

	private:
		void incRef() {
			if (resource == nullptr) return;
			loader->incReference(resource);
		}

		void decRef() {
			if (resource == nullptr) return;
			loader->decReference(resource);
		}

		T* resource;
		ResourceLoader* loader;
	};

	/**
	 * Resource manager interface.
	 * Used to load and handle each resource type.
	 * Usually a ResourceManager takes care of loading one type of resource.
	 * Like Textures, Shaders, Models, etc.
	 * It's possible that a ResourceManager will ask for resources from different managers.
	 * e.g: The ModelManager asks for Textures which the TextureManager loaded.
	 */
	class ResourceManager
	{
		public:
			virtual ~ResourceManager(){}

			virtual bool init() = 0;
			virtual void update() = 0;

			virtual void setResourceLoader(ResourceLoader* loader) = 0;
			virtual BaseResource* loadResource(ResourceMeta* meta) = 0;
			
			// Called when the given Resource Meta is not in use anymore
			// This will only be called after a previous call to loadResource() was made
			// Release the resource or not is up to the implementation
			// Usually cheap resources are kept in memory (may be released from drivers)
			virtual void resourceNotInUse(ResourceMeta* meta) = 0;

			virtual void reloadResource(BaseResource* resource) = 0;

			// Called after a while when the resource is not being used for some time.
			// After this call it's expected that most memory used by the resource is freed.
			// All resource references must still be valid, since we're just requesting the memory
			// for the resource to be released.
			virtual void resourceRelease(ResourceMeta* meta) = 0;

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
