#pragma once

#include <string>
#include <vector>

#include "Common/TypeDefinition.h"
#include "Common/ThreadSafeQueue.h"
#include "Core/Resources/ResourceIndexer.h"
#include "Core/Logger.h"

#include "json.h"

namespace BitEngine {

	class ResourceManager;

	// All resources types should come from this
	class BaseResource
	{
		public:
			BaseResource()
				: resourceId(0)
			{}

			// Base resource
			// d Owns the data from this vector
			BaseResource(uint32 id, const std::string& p, std::vector<char>& d)
				: resourceId(id), path(p)
			{
				data.swap(d);
			}

			// Takes ownership of pointer
			void set(uint32 id, const std::string& p, std::vector<char>& d)
			{
				resourceId = id;
				path = p;
				data.swap(d);
			}

			void clearBaseData()
			{
				std::vector<char>().swap(data);
			}

			const std::string& getPath() const
			{
				return path;
			}

			uint32 getResourceId() const {
				return resourceId;
			}

		protected:
			// The resource data in memory.
			// Not all resources need to stay in memory to be used (Textures may stay on GPU)
			// This pointer when loaded by the ResourceLoader may not be the same after passing through a Resource Manager
			// A loader may get a compressed file from disk and later the resource manager may uncompress it and
			// save this reference instead.
			// Example:
			// After loading a .png image the TextureManager uncompress it and get it's pixel data.
			// Loads the pixel data to GPU, free the original file data and
			// leaves the unconpressed pixel data in memory for faster reloads in case the texture
			// was removed from the GPU memory
			std::vector<char> data;

			// Unique global resource id inside the ResourceLoader
			// this resource was loaded from
			uint32 resourceId;

			// Original path name used in the request for loading this resource
			std::string path;
	};

	typedef nlohmann::json ResourceProperty;

	struct ResourceMeta
	{
		ResourceMeta(const std::string& pack)
			: id(~0), package(pack)
		{}

		const std::string toString() const;

		uint32 id;
		std::string package;
		std::string resourceName;
		std::string type;
		ResourceProperty properties;
	};

	// Resource Loader interface
	// Used by the application to retrieve the final resource.
	class ResourceLoader
	{
		public:
		    virtual ~ResourceLoader(){}

			virtual bool init() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void registerResourceManager(const std::string& resourceType, ResourceManager* manager) = 0;

			// Blocking call
			// Read a file and map all indices
			// If any resource override would happen, they're skipped
			// and a warning is logged.
			virtual void loadIndex(const std::string& index) = 0;
			
			virtual void releaseAll() = 0;
			virtual void releaseResource(uint32 id) = 0;

			template<typename T>
			T* getResource(const std::string& name) {
				return static_cast<T*>(loadResource(name));
			}

			// Returns the request ID
			// Queue the request to be loaded by another thread
			//virtual uint32 loadRequest(const std::string& path, BaseResource* into, ResourceManager* callback) = 0;
			//virtual bool reloadResource(uint32 resourceID, ResourceManager* callback) = 0;

			virtual void waitForAll() = 0;
			virtual void waitForResource(BaseResource* resource) = 0;

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

			struct DataRequest 
			{
				enum LoadState {
					NOT_LOADED,
					LOADING,
					LOADED,
					ERROR,
				};
				DataRequest(const DataRequest& dr)
					: meta(dr.meta), loadState(dr.loadState), data(dr.data)
				{
					LOG(EngineLog, BE_LOG_WARNING) << "USE MOVE INSTEAD!";
				}
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

			// Used internally by resource managers, to retrieve raw resource data
			// Like, texture image data, sound data, and others.
			virtual void requestResourceData(ResourceMeta* meta, ThreadSafeQueue<DataRequest>* responseTo) = 0;
	};

	// Resource manager interface
	// Used to load and handle each resource type
	class ResourceManager
	{
		public:
			virtual bool init() = 0;

			virtual void update() = 0;

			virtual BaseResource* loadResource(ResourceMeta* base) = 0;

			virtual void onResourceLoaded(uint32 resourceID) = 0;
			virtual void onResourceLoadFail(uint32 resourceID) = 0;

			// in bytes
			virtual uint32 getCurrentRamUsage() const = 0;
			virtual uint32 getCurrentGPUMemoryUsage() const = 0;
	};

}
