#include <type_traits>
#include "frame.hpp"

namespace visionaray {

    Frame::Frame()
        : resourceHandle(new std::remove_pointer_t<ANARIFrame>)
    {
    }

    Frame::~Frame()
    {
    }

    const uint8_t* Frame::map(const char* channel)
    {
        // TODO:
        static uint8_t* data = new uint8_t[1024*1024];
        return data;
    }

    ResourceHandle Frame::getResourceHandle()
    {
        return resourceHandle;
    }
} // visionaray


