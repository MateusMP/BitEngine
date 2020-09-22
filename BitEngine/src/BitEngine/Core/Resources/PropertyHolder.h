#pragma once

#include <vector>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Math.h"

namespace BitEngine {
class ResourceLoader;

template <typename T>
class RR;

struct EnumStruct {
    operator u32()
    {
        return value;
    }

    u32 value = 0;
};

/***
 * Stores resource meta data.
 */
class BE_API ResourceMeta {
public:
    ResourceMeta()
        : id(~0)
        , references(0)
    {
    }

    const std::string toString() const
    {
        return "Resource: " + std::to_string(id);
    }

    u32 getReferences() const
    {
        return references;
    }

    std::string getNameId() const
    {
        return std::to_string(id);
    }

    u32 id;

private:
    friend class ResourceLoader;
    u32 references;
};

/**
 * All resources types should come from this
 */
class BE_API BaseResource {
public:
    // Base resource
    // d Owns the data from this vector
    BaseResource(ResourceMeta* _meta)
        : meta(_meta)
    {
    }

    u32 getResourceId() const
    {
        return meta->id;
    }

    ResourceMeta* getMeta() const
    {
        return meta;
    }

    void setMeta(ResourceMeta* m)
    {
        meta = m;
    }

protected:
    ResourceMeta* meta;
};

class PropertyHolder {
public:
    template <typename T>
    void readObject(const char* name, T* into)
    {
        PropertyHolder* props = getReaderObject(name);
        T::read(props, into);
    }

    template <typename T>
    void readCustom(const char* field, T* into)
    {
        T::read(field, this, into);
    }

    template <typename T>
    void readObjectFromList(const char* name, ptrsize index, T* into)
    {
        PropertyHolder* props = getReaderObjectFromList(name, index);
        T::read(props, into);
    }

    template <typename T>
    void readObjectList(const char* name, std::vector<T>* into)
    {
        ptrsize size = getPropertyListSize(name);
        into->resize(size);
        for (ptrsize i = 0; i < size; ++i) {
            readObjectFromList(name, i, &into->at(i));
        }
    }

    template <typename T>
    void read(const char* name, T* type)
    {
        _read(name, type);
    }

    void read(const char* name, EnumStruct* type)
    {
        _read(name, &type->value);
    }

    template <typename T>
    void read(const char* name, RR<T>* into)
    {
        BaseResource* res = nullptr;
        _read(name, &res);
        if (res) {
            *into = RR<T>((T*)res, getLoader());
        }
        else {
            *into = RR<T>();
        }
    }

    virtual ResourceLoader* getLoader() = 0;

    virtual ptrsize getPropertyListSize(const char* name) = 0;
    virtual PropertyHolder* getReaderObject(const char* name) = 0;
    virtual PropertyHolder* getReaderObjectFromList(const char* name, ptrsize index) = 0;

protected:
    virtual void _read(const char* name, u32* into) = 0;
    virtual void _read(const char* name, s32* into) = 0;
    virtual void _read(const char* name, float* into) = 0;
    virtual void _read(const char* name, double* into) = 0;
    virtual void _read(const char* name, std::string* into) = 0;
    virtual void _read(const char* name, BaseResource** into) = 0;
    virtual void _read(const char* name, ResourceMeta** into) = 0;
    virtual void _read(const char* name, Vec3* into) = 0;
    virtual void _read(const char* name, Vec4* into) = 0;
};

template <>
void PropertyHolder::read<u8>(const char* name, u8* type);
}