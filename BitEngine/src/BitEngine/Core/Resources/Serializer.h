#pragma once


class Serializer {
    template<typename T>
    void read(const char* name, const T t);

    template<typename T>
    void write(const char* name, const T t);
};