#include "array.hpp"
#include "backend.hpp"
#include "group.hpp"
#include "logging.hpp"

namespace generic {

    Group::Group()
        : resourceHandle(new std::remove_pointer_t<ANARIGroup>)
    {
    }

    Group::~Group()
    {
    }

    ResourceHandle Group::getResourceHandle()
    {
        return resourceHandle;
    }

    void Group::commit()
    {
        backend::commit(*this);
    }

    void Group::release()
    {
    }

    void Group::retain()
    {
    }

    void Group::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"surface",7)==0 && type==ANARI_ARRAY1D) {
            surface = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"volume",6)==0 && type==ANARI_ARRAY1D) {
            volume = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"light",5)==0 && type==ANARI_ARRAY1D) {
            light = *(ANARIArray1D*)mem; // TODO: reference count
        } else {
            LOG(logging::Level::Warning) << "Group: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


