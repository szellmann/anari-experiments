#pragma once

#include "geometry.hpp"

namespace generic {

    class SphereGeom : public Geometry
    {
    public:
        SphereGeom();
       ~SphereGeom();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        ANARIArray1D vertex_position = nullptr;
        ANARIArray1D vertex_radius = nullptr;
        ANARIArray1D vertex_color = nullptr;
        ANARIArray1D vertex_attribute0 = nullptr;
        ANARIArray1D vertex_attribute1 = nullptr;
        ANARIArray1D vertex_attribute2 = nullptr;
        ANARIArray1D vertex_attribute3 = nullptr;
        ANARIArray1D primitive_index = nullptr;
        ANARIArray1D primitive_radius = nullptr;
        float radius = 1.f;
       
    };

} // generic


