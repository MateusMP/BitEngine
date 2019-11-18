#pragma once

#include <string>
#include <vector>
#include <map>
#include <type_traits>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Common/ThreadSafeQueue.h"
#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/Task.h"
#include "BitEngine/Core/Resources/ResourceLoader.h"

namespace BitEngine {

/**
 * Resource manager interface.
 * Used to load and handle each resource type.
 * Usually a ResourceManager takes care of loading one type of resource.
 * Like Textures, Shaders, Models, etc.
 * It's possible that a ResourceManager will ask for resources from different managers.
 * e.g: The ModelManager asks for Textures which the TextureManager loaded.
 */
class BE_API ResourceManager
{
public:
    virtual ~ResourceManager() {}

    virtual bool init() = 0;
    virtual void update() = 0;

    // Should release ALL resources
    virtual void shutdown() = 0;

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

};

}
