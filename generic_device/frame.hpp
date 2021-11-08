#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Frame : public Object
    {
    public:
        Frame();
       ~Frame();

        const void* map(const char* channel);

        ResourceHandle getResourceHandle();

        void commit();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        ANARIWorld world = nullptr;
        ANARICamera camera = nullptr;
        ANARIRenderer renderer = nullptr;
        unsigned size[2] = {0,0};
        ANARIDataType color;
        ANARIDataType depth;

    private:
        ANARIFrame resourceHandle = nullptr;
    };

} // generic


