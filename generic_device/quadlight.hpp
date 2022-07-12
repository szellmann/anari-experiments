#pragma once

#include "light.hpp"

namespace generic {

    class QuadLight : public Light
    {
    public:
        QuadLight();
       ~QuadLight();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        float position[3] = {0.f,0.f,0.f};
        float edge1[3] = {1.f,0.f,0.f};
        float edge2[3] = {0.f,1.f,0.f};
        float intensity = 1.f;
        float power = 1.f;
        float radiance = 1.f;
        char side[256] = "front";
        // intensity, if set, takes precedence over power
        bool intensityWasSet = false;
        // power, if set, takes precedence over radiance
        bool powerWasSet = false;
    };

} // generic


