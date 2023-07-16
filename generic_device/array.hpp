#pragma once

#include <cstddef>
#include <cstdint>
#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class ArrayStorage : public Resource
    {
    public:
        ArrayStorage(const void* appMemory, ANARIMemoryDeleter deleter,
                     const void* userPtr, ANARIDataType elementType);
        virtual ~ArrayStorage();

        virtual ResourceHandle getResourceHandle() = 0;

        void commit();

        void release();

        void retain();

        virtual size_t getSizeInBytes() const = 0;

        const void* appMemory = nullptr;
        ANARIMemoryDeleter deleter = nullptr;
        const void* userPtr = nullptr; // additional pointer, can be passed to deleter
        uint8_t* internalData;
        ANARIDataType elementType;
    };

    class Array1D : public ArrayStorage
    {
    public:
        Array1D(const void* appMemory, ANARIMemoryDeleter deleter, const void* userPtr,
                ANARIDataType elementType, uint64_t numItems1);
       ~Array1D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[1] = {1};

    private:
        size_t getSizeInBytes() const;

        ANARIArray1D resourceHandle = nullptr;
    };


    class Array2D : public ArrayStorage
    {
    public:
        Array2D(const void* appMemory, ANARIMemoryDeleter deleter, const void* userPtr,
                ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2);
       ~Array2D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[2] = {1,1};

    private:
        size_t getSizeInBytes() const;

        ANARIArray2D resourceHandle = nullptr;
    };


    class Array3D : public ArrayStorage
    {
    public:
        Array3D(const void* appMemory, ANARIMemoryDeleter deleter, const void* userPtr,
                ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
                uint64_t numItems3);
       ~Array3D();

        ResourceHandle getResourceHandle();

        uint64_t numItems[3] = {1,1,1};

    private:
        size_t getSizeInBytes() const;

        ANARIArray3D resourceHandle = nullptr;
    };
} // generic


