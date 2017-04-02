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
#include "bitengine/Core/Resources/ResourceLoader.h"

namespace BitEngine {

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
