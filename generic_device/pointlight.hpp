#pragma once

#include "light.hpp"

namespace generic {

    class PointLight : public Light
    {
    public:
        PointLight();
       ~PointLight();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        float position[3];
        float intensity = 1.f;
        float power = 1.f;

    private:
        // intensity, if set, takes precidence over power
        bool intensityWasSet = false;
    };

} // generic


