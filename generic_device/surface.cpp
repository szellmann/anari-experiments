#include <string.h>
#include <type_traits>
#include "logging.hpp"
#include "surface.hpp"

namespace generic {

    Surface::Surface()
        : resourceHandle(new std::remove_pointer_t<ANARISurface>)
    {
    }

    Surface::~Surface()
    {
    }

    ResourceHandle Surface::getResourceHandle()
    {
        return resourceHandle;
    }

    void Surface::commit()
    {
        if (geometry == nullptr)
            LOG(logging::Level::Error) << "Surface error: geometry not set but is a "
                << "required parameter";
    }

    void Surface::release()
    {
    }

    void Surface::retain()
    {
    }

    void Surface::setParameter(const char* name,
                               ANARIDataType type,
                               const void* mem)
    {
        if (strncmp(name,"geometry",8)==0 && type==ANARI_GEOMETRY) {
            geometry = *(ANARIGeometry*)mem; // TODO: reference count
        } else if (strncmp(name,"material",8)==0 && type==ANARI_MATERIAL) {
            material = *(ANARIMaterial*)mem; // TODO: reference count
        } else {
            LOG(logging::Level::Warning) << "Surface: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


