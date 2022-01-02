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

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        int getProperty(const char* name,
                        ANARIDataType type,
                        void* mem,
                        uint64_t size,
                        uint32_t waitMask);

        ANARIWorld world = nullptr;
        ANARICamera camera = nullptr;
        ANARIRenderer renderer = nullptr;
        unsigned size[2] = {0,0};
        ANARIDataType color;
        ANARIDataType depth;

        std::future<void> renderFuture;

        // last duration for rendering
        float duration = 0.f;

    private:
        ANARIFrame resourceHandle = nullptr;
    };

} // generic


