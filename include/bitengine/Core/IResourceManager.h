#pragma once

#include <string>
#include <vector>

#include "Common/TypeDefinition.h"

namespace BitEngine {

	class IResourceManager;

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

	// Resource Loader interface
	class IResourceLoader
	{
		public:
		    virtual ~IResourceLoader(){}

			virtual void Shutdown() = 0;

			virtual void releaseAll() = 0;
			virtual void releaseResource(uint32 id) = 0;

			// Returns the request ID
			// Queue the request to be loaded by another thread
			virtual uint32 loadRequest(const std::string& path, BaseResource* into, IResourceManager* callback) = 0;
			virtual bool reloadResource(uint32 resourceID, IResourceManager* callback) = 0;

			virtual void waitForAll() = 0;
	};

	// Resource manager interface
	class IResourceManager
	{
		public:
			virtual bool Init() = 0;

			virtual void setResourceLoader(IResourceLoader* loader) = 0;

			virtual void onResourceLoaded(uint32 resourceID) = 0;
			virtual void onResourceLoadFail(uint32 resourceID) = 0;

			// in bytes
			virtual uint32 getCurrentRamUsage() const = 0;
			virtual uint32 getCurrentGPUMemoryUsage() const = 0;

		private:

	};

}
