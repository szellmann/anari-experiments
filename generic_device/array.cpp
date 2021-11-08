#include <anari/detail/Helpers.h>
#include "array.hpp"
#include <iostream>

namespace generic {

    //--- ArrayStorage ------------------------------------
    ArrayStorage::ArrayStorage(uint8_t* userPtr, ANARIMemoryDeleter deleter,
                               void* userdata, ANARIDataType elementType)
        : userPtr(userPtr)
        , deleter(deleter)
        , userdata(userdata)
        , elementType(elementType)
    {
    }

    ArrayStorage::~ArrayStorage()
    {
        free();
    }

    void ArrayStorage::commit()
    {
        if (0)//deleter != nullptr)
            data = userPtr;
        else
            {} // would handle managed memory here
    }

    void ArrayStorage::alloc()
    {
        if (deleter == nullptr)
            this->data = new uint8_t[getSizeInBytes()];
    }

    void ArrayStorage::free()
    {
        if (userPtr != data)
            delete[] data;
        else if (data != nullptr)
            deleter(data,userdata);
        data = nullptr;
    }

    //--- Array2D -----------------------------------------
    Array1D::Array1D(uint8_t* userPtr, ANARIMemoryDeleter deleter, void* userdata,
          ANARIDataType elementType, uint64_t numItems1, uint64_t byteStride1)
        : ArrayStorage(userPtr,deleter,userdata,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray1D>)
    {
        numItems[0] = numItems1;

        byteStride[0] = byteStride1;

        ArrayStorage::alloc();
    }

    Array1D::~Array1D()
    {
        ArrayStorage::free();
    }

    ResourceHandle Array1D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array1D::getSizeInBytes() const
    {
        return numItems[0]*anari::sizeOfDataType(elementType);
    }

    //--- Array2D -----------------------------------------
    Array2D::Array2D(uint8_t* userPtr, ANARIMemoryDeleter deleter, void* userdata,
          ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
          uint64_t byteStride1, uint64_t byteStride2)
        : ArrayStorage(userPtr,deleter,userdata,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray2D>)
    {
        numItems[0] = numItems1;
        numItems[1] = numItems2;

        byteStride[0] = byteStride1;
        byteStride[1] = byteStride2;

        ArrayStorage::alloc();
    }

    Array2D::~Array2D()
    {
        ArrayStorage::free();
    }

    ResourceHandle Array2D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array2D::getSizeInBytes() const
    {
        return numItems[0]*numItems[1]*anari::sizeOfDataType(elementType);
    }

    //--- Array3D -----------------------------------------
    Array3D::Array3D(uint8_t* userPtr, ANARIMemoryDeleter deleter, void* userdata,
          ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
          uint64_t numItems3, uint64_t byteStride1, uint64_t byteStride2,
          uint64_t byteStride3)
        : ArrayStorage(userPtr,deleter,userdata,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray3D>)
    {
        numItems[0] = numItems1;
        numItems[1] = numItems2;
        numItems[2] = numItems3;

        byteStride[0] = byteStride1;
        byteStride[1] = byteStride2;
        byteStride[2] = byteStride3;

        ArrayStorage::alloc();
    }

    Array3D::~Array3D()
    {
        ArrayStorage::free();
    }

    ResourceHandle Array3D::getResourceHandle()
    {
        return resourceHandle;
    }

    size_t Array3D::getSizeInBytes() const
    {
        return numItems[0]*numItems[1]*numItems[2]*anari::sizeOfDataType(elementType);
    }

} // generic


