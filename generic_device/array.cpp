#include <string.h>
#include <anari/anari_cpp.hpp>
#include "array.hpp"
#include <iostream>

namespace generic {

    //--- ArrayStorage ------------------------------------
    ArrayStorage::ArrayStorage(const void* appMemory, ANARIMemoryDeleter deleter,
                               const void* userPtr, ANARIDataType elementType)
        : appMemory(appMemory)
        , deleter(deleter)
        , userPtr(userPtr)
        , elementType(elementType)
    {
        // if this is NULL, we'll later allocate storage from
        // within the implementation classes and make the array
        // managed
        internalData = (uint8_t*)appMemory;
    }

    ArrayStorage::~ArrayStorage()
    {
        if (appMemory == nullptr) {
            // that means the array is managed, so we delete the data
            delete[] internalData;
        }
    }

    void ArrayStorage::commit()
    {
    }

    void ArrayStorage::release()
    {
        // Transfer ownership after user releases the object
        if (appMemory != nullptr) {
            internalData = new uint8_t[getSizeInBytes()];
            memcpy(internalData,appMemory,getSizeInBytes());

            if (deleter != nullptr)
                deleter(userPtr,appMemory);
        }
    }

    void ArrayStorage::retain()
    {
    }

    //--- Array1D -----------------------------------------
    Array1D::Array1D(const void* appMemory, ANARIMemoryDeleter deleter,
          const void* userPtr, ANARIDataType elementType, uint64_t numItems1)
        : ArrayStorage(appMemory,deleter,userPtr,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray1D>)
    {
        numItems[0] = numItems1;

        if (internalData == nullptr)
            internalData = new uint8_t[getSizeInBytes()];
    }

    Array1D::~Array1D()
    {
    }

    ResourceHandle Array1D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array1D::getSizeInBytes() const
    {
        return numItems[0]*anari::sizeOf(elementType);
    }

    //--- Array2D -----------------------------------------
    Array2D::Array2D(const void* appMemory, ANARIMemoryDeleter deleter,
          const void* userPtr, ANARIDataType elementType,uint64_t numItems1,
          uint64_t numItems2)
        : ArrayStorage(appMemory,deleter,userPtr,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray2D>)
    {
        numItems[0] = numItems1;
        numItems[1] = numItems2;

        if (internalData == nullptr)
            internalData = new uint8_t[getSizeInBytes()];
    }

    Array2D::~Array2D()
    {
    }

    ResourceHandle Array2D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array2D::getSizeInBytes() const
    {
        return numItems[0]*numItems[1]*anari::sizeOf(elementType);
    }

    //--- Array3D -----------------------------------------
    Array3D::Array3D(const void* appMemory, ANARIMemoryDeleter deleter,
          const void* userPtr, ANARIDataType elementType, uint64_t numItems1,
          uint64_t numItems2, uint64_t numItems3)
        : ArrayStorage(appMemory,deleter,userPtr,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray3D>)
    {
        numItems[0] = numItems1;
        numItems[1] = numItems2;
        numItems[2] = numItems3;

        if (internalData == nullptr)
            internalData = new uint8_t[getSizeInBytes()];
    }

    Array3D::~Array3D()
    {
    }

    ResourceHandle Array3D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array3D::getSizeInBytes() const
    {
        return numItems[0]*numItems[1]*numItems[2]*anari::sizeOf(elementType);
    }

} // generic


