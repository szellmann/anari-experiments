#pragma once

#include <cstdint>
#include "object.hpp"
#include "resource.hpp"

namespace visionaray {

    class Frame : public Object
    {
    public:
        Frame();
       ~Frame();

        const uint8_t* map(const char* channel);

        ResourceHandle getResourceHandle();

        void commit();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

    private:
        ANARIFrame resourceHandle = nullptr;
    };

} // visionaray


