#pragma once

#include <cstdint>
#include "spatialfield.hpp"

namespace generic {

    class StructuredRegular : public SpatialField
    {
    public:
        StructuredRegular();
       ~StructuredRegular();

        void commit();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        ANARIArray3D data = nullptr;
        float origin[3] = {0,0,0};
        float spacing[3] = {1,1,1};
        const char* filter = "linear";

    };

} // generic


