#pragma once

#include "geometry.hpp"

namespace generic {

    class TriangleGeom : public Geometry
    {
    public:
        TriangleGeom();
       ~TriangleGeom();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

    };

} // generic


