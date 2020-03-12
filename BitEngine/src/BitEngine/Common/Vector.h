#pragma once

#include <type_traits>

#include "BitEngine/Core/Memory.h"

namespace BitEngine {

template<typename T, ptrsize MaxSize, bool Local>
class _TightFixedVector {};

template<typename T, ptrsize MaxSize>
class _TightFixedVector<T, MaxSize, false>
{
protected:
    ptrsize count = 0;
    T* data;
};
template<typename T, ptrsize MaxSize>
class _TightFixedVector<T, MaxSize, true>
{
protected:
    ptrsize count = 0;
    T data[MaxSize];
};

template<typename T, ptrsize MaxSize, bool Local = true>
class BE_API TightFixedVector : public _TightFixedVector<T, MaxSize, Local> {
public:
    template<bool enabled = Local>
    static void Create(typename std::enable_if<enabled, MemoryArena&>::type a) {
    }

    template<bool enabled = Local>
    static TightFixedVector<T, MaxSize, Local>&& Create(typename std::enable_if<!enabled, MemoryArena&>::type arena)
    {
        TightFixedVector<T, MaxSize, Local> fv;
        fv.data = arena.template pushArray<T, MaxSize>();
        fv.count = 0;
        return std::move(fv);
    }

    class iterator {
    public:
        iterator(T* ptr) : ptr(ptr) {}
        iterator operator++() { ++ptr; return *this; }
        bool operator!=(const iterator & other) const { return ptr != other.ptr; }
        const T& operator*() const { return *ptr; }
        T* operator->() {
            return ptr;
        }
    protected:
        friend class TightFixedVector;
        T* ptr;
    };

    T& operator[](ptrsize s) {
        return this->data[s];
    }

    void push_back(T&& t) {
        emplace_back(t);
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        BE_ASSERT(this->count < MaxSize);
        this->data[this->count] = T(std::forward<Args>(args)...);
        ++this->count;
    }

    void clear() {
        this->count = 0;
    }

    // When the order doesnt matter
    void remove(ptrsize index) {
        if (index < this->count) {
            --this->count;
            if (index != this->count) {
                this->data[index] = this->data[this->count];
            }
            this->data[this->count] = {};
        } else {
            --this->count;
            this->data[index] = {};
        }
    }

    void erase(iterator it) {
        const ptrsize index = it.ptr - this->data;
        remove(index);
    }

    iterator begin() { return iterator(this->data); }
    iterator end() { return iterator(this->data + this->count); }

};

}