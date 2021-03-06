#pragma once

#include <math.h>
#include "camera.hpp"

namespace generic {

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera();
       ~PerspectiveCamera();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        float fovy = M_PI/3.f;
        float aspect = 1.f;
    };

} // generic


