#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Group : public Object
    {
    public:
        Group();
       ~Group();

        ResourceHandle getResourceHandle();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        ANARIArray1D surface = nullptr;
        ANARIArray1D volume = nullptr;
        ANARIArray1D light = nullptr;
        // TODO: bounds

    private:
        ANARIGroup resourceHandle;
    };

} // generic


