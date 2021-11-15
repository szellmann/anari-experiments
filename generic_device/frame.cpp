#include <string.h>
#include <type_traits>
#include "backend.hpp"
#include "frame.hpp"
#include "logging.hpp"
#include "frame.hpp"

namespace generic {

    Frame::Frame()
        : resourceHandle(new std::remove_pointer_t<ANARIFrame>)
    {
    }

    Frame::~Frame()
    {
    }

    const void* Frame::map(const char* channel)
    {
        return backend::map(*this);
    }

    int Frame::wait(ANARIWaitMask m)
    {
        return backend::wait(*this,m);
    }

    ResourceHandle Frame::getResourceHandle()
    {
        return resourceHandle;
    }

    void Frame::commit()
    {
        backend::commit(*this);
    }

    void Frame::release()
    {
    }

    void Frame::retain()
    {
    }

    void Frame::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"world",5)==0 && type==ANARI_WORLD) {
            world = *(ANARIWorld*)mem; // TODO: reference count
        } else if (strncmp(name,"camera",6)==0 && type==ANARI_CAMERA) {
            camera = *(ANARICamera*)mem; // TODO: reference count
        } else if (strncmp(name,"renderer",8)==0 && type==ANARI_RENDERER) {
            renderer = *(ANARIRenderer*)mem; // TODO: reference count
        } else if (strncmp(name,"size",4)==0 && type==ANARI_UINT32_VEC2) {
            memcpy(size,mem,sizeof(size));
        } else if (strncmp(name,"color",5)==0 && type==ANARI_DATA_TYPE) {
            memcpy(&color,mem,sizeof(color));
        } else if (strncmp(name,"depth",5)==0 && type==ANARI_DATA_TYPE) {
            memcpy(&depth,mem,sizeof(depth));
        } else {
            LOG(logging::Level::Warning) << "Frame: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


