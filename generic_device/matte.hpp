#pragma once

#include "material.hpp"

namespace generic {

    class Matte : public Material
    {
    public:
        Matte();
       ~Matte();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        float color[3] = {.8f,.8f,.8f};
    };

} // generic


