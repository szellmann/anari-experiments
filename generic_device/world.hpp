#pragma once

#include "object.hpp"
#include "resource.hpp"
#include "volume.hpp"

namespace generic {

    class World : public Object
    {
    public:
        World();
       ~World();

        ResourceHandle getResourceHandle();

        void commit();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        ANARIArray1D volume = nullptr;

    private:
        ANARIWorld resourceHandle;
    };

} // generic


