#pragma once

#include <cstring>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Resources/ResourceLoader.h"

namespace BitEngine {

class File : public BaseResource
{
public:
    File() : BaseResource(nullptr), data(nullptr), size(0), ready(false)
    {}
    File(ResourceMeta* meta) : BaseResource(meta),
        data(nullptr), size(0), ready(false)
    {}

    void* data;
    u32 size;
    bool ready;
};

}