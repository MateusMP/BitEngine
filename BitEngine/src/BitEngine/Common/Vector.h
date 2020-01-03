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
class TightFixedVector : public _TightFixedVector<T, MaxSize, Local> {
public:
    template<bool enabled = Local>
    static void Create(typename std::enable_if<enabled, MemoryArena&>::type a) {
    }

    template<bool enabled = Local>
    static TightFixedVector<T, MaxSize, Local>&& Create(typename std::enable_if<!enabled, MemoryArena&>::type a)
    {
        TightFixedVector<T, MaxSize, Local> fv;
        fv.data = arena.pushArray<T, MaxSize>();
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
        return data[s];
    }

    void push_back(T&& t) {
        emplace_back(t);
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        BE_ASSERT(count < MaxSize);
        data[count] = T(std::forward<Args>(args)...);
        ++count;
    }

    void clear() {
        count = 0;
    }

    // When the order doesnt matter
    void remove(ptrsize index) {
        if (index < count) {
            --count;
            data[index] = data[count];
            data[count].~T();
        } else {
            --count;
            data[index].~T();
        }
    }

    void erase(iterator it) {
        const ptrsize index = it.ptr - data;
        remove(index);
    }

    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data + count); }

};

}