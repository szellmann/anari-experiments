#pragma once

#include <cstddef>
#include <cstdint>
#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class ArrayStorage : public Resource
    {
    public:
        ArrayStorage(const void* userPtr, ANARIMemoryDeleter deleter,
                     const void* userdata, ANARIDataType elementType);
       ~ArrayStorage();

        virtual ResourceHandle getResourceHandle() = 0;

        void commit();

        void release();

        void retain();

        virtual size_t getSizeInBytes() const = 0;

        void alloc();

        void free();

        const void* userPtr = nullptr;
        uint8_t* data = nullptr; // initially set to userPtr, managed on app release
        ANARIMemoryDeleter deleter = nullptr;
        const void* userdata = nullptr; // for deleter
        ANARIDataType elementType;
    };

    class Array1D : public ArrayStorage
    {
    public:
        Array1D(const void* data, ANARIMemoryDeleter deleter, const void* userdata,
                ANARIDataType elementType, uint64_t numItems1, uint64_t buyteStride1);
       ~Array1D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[1] = {1};
        uint64_t byteStride[1] = {0};

    private:
        size_t getSizeInBytes() const;

        ANARIArray1D resourceHandle = nullptr;
    };


    class Array2D : public ArrayStorage
    {
    public:
        Array2D(const void* data, ANARIMemoryDeleter deleter, const void* userdata,
                ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
                uint64_t byteStride1, uint64_t byteStride2);
       ~Array2D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[2] = {1,1};
        uint64_t byteStride[2] = {0,0};

    private:
        size_t getSizeInBytes() const;

        ANARIArray2D resourceHandle = nullptr;
    };


    class Array3D : public ArrayStorage
    {
    public:
        Array3D(const void* data, ANARIMemoryDeleter deleter, const void* userdata,
                ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
                uint64_t numItems3, uint64_t byteStride1, uint64_t byteStride2,
                uint64_t byteStride3);
       ~Array3D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[3] = {1,1,1};
        uint64_t byteStride[3] = {0,0,0};

    private:
        size_t getSizeInBytes() const;

        ANARIArray3D resourceHandle = nullptr;
    };
} // generic


