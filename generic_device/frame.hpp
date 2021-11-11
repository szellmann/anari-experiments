#pragma once

#include <future>
#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Frame : public Object
    {
    public:
        Frame();
       ~Frame();

        const void* map(const char* channel);

        int wait(ANARIWaitMask m);

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

        std::future<void> renderFuture;

    private:
        ANARIFrame resourceHandle = nullptr;
    };

} // generic


