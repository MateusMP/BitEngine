#pragma once

#include <string>

#include "Common/TypeDefinition.h"

namespace BitEngine {

	class IResourceManager;

	// All resources come from this
	class BaseResource
	{
		public:
			BaseResource()
				: resourceId(0), dataPtr(0), dataSize(0)
			{}

			BaseResource(uint32 id, const std::string& p, char* data, int size)
				: resourceId(id), path(p), dataPtr(data), dataSize(size)
			{}

			// Unique global resource id inside the ResourceLoader
			// this resource was loaded from
			uint32 resourceId;

			// Original path name used in the request for loading this resource
			std::string path;

			// TODO: add extra resource information retrieved from package data?

			// Takes owner ship of pointer
			void setBaseData(char* data, int size)
			{
				dataPtr = data;
				dataSize = size;
			}

			void clearBaseData()
			{
				if (dataPtr)
					delete[] dataPtr;
				dataPtr = nullptr;
				dataSize = 0;
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
			char* dataPtr;

			// Number of bytes pointed by dataPtr.
			// This may be updated by the ResourceManager
			int dataSize;
	};

	// Load a resource from disk or package
	class IResourceLoader
	{
		public:
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
			virtual uint32 getCurrentRamUsage() = 0;
			virtual uint32 getCurrentGPUMemoryUsage() = 0;

		private:

	};

}
