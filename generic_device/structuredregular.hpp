#pragma once

#include "spatialfield.hpp"

namespace generic {

    class StructuredRegular : public SpatialField
    {
    public:
        StructuredRegular();
       ~StructuredRegular();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        ANARIArray3D data = nullptr;
        float origin[3] = {0,0,0};
        float spacing[3] = {1,1,1};
        const char* filter = "linear";

    };

} // generic


