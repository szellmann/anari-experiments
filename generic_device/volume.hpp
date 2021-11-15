#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Volume : public Object
    {
    public:
        Volume();
       ~Volume();

        ResourceHandle getResourceHandle();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        ANARISpatialField field = nullptr;
        float valueRange[2] = {0.f,1.f};
        ANARIArray1D color = nullptr;
        ANARIArray1D color_position = nullptr;
        ANARIArray1D opacity = nullptr;
        ANARIArray1D opacity_position = nullptr;
        float densityScale = 1.f;

    private:
        ANARIVolume resourceHandle;
    };

    std::unique_ptr<Volume> createVolume(const char* subtype);

} // generic


