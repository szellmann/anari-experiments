#include <string.h>
#include <anari/detail/helpers.h>
#include "array.hpp"
#include <iostream>

namespace generic {

    //--- ArrayStorage ------------------------------------
    ArrayStorage::ArrayStorage(void* userPtr, ANARIMemoryDeleter deleter,
                               void* userdata, ANARIDataType elementType)
        : userPtr(userPtr)
        , data((uint8_t*)userPtr)
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
    }

    void ArrayStorage::release()
    {
        // Transfer ownership after user releases the object
        // TODO: make sure there are no user references anymore
        data = new uint8_t[getSizeInBytes()];
        memcpy(data,userPtr,getSizeInBytes());
        if (deleter != nullptr)
            deleter(userPtr,userdata);
    }

    void ArrayStorage::retain()
    {
    }

    void ArrayStorage::alloc()
    {
    }

    void ArrayStorage::free()
    {
        if (userPtr != data)
            delete[] data;
        else if (data != nullptr && deleter != nullptr)
            deleter(data,userdata);
        data = nullptr;
    }

    //--- Array1D -----------------------------------------
    Array1D::Array1D(void* userPtr, ANARIMemoryDeleter deleter, void* userdata,
          ANARIDataType elementType, uint64_t numItems1, uint64_t byteStride1)
        : ArrayStorage(userPtr,deleter,userdata,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray1D>)
    {
        numItems[0] = numItems1;

        byteStride[0] = byteStride1;
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
    Array2D::Array2D(void* userPtr, ANARIMemoryDeleter deleter, void* userdata,
          ANARIDataType elementType, uint64_t numItems1, uint64_t numItems2,
          uint64_t byteStride1, uint64_t byteStride2)
        : ArrayStorage(userPtr,deleter,userdata,elementType)
        , resourceHandle(new std::remove_pointer_t<ANARIArray2D>)
    {
        numItems[0] = numItems1;
        numItems[1] = numItems2;

        byteStride[0] = byteStride1;
        byteStride[1] = byteStride2;
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
    Array3D::Array3D(void* userPtr, ANARIMemoryDeleter deleter, void* userdata,
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


