#include <string.h>
#include <type_traits>
#include "instance.hpp"
#include "logging.hpp"

namespace generic {

    Instance::Instance()
        : resourceHandle(new std::remove_pointer_t<ANARIInstance>)
    {
    }

    Instance::~Instance()
    {
    }

    ResourceHandle Instance::getResourceHandle()
    {
        return resourceHandle;
    }

    void Instance::commit()
    {
    }

    void Instance::release()
    {
    }

    void Instance::retain()
    {
    }

    void Instance::setParameter(const char* name,
                                ANARIDataType type,
                                const void* mem)
    {
        if (strncmp(name,"group",5)==0 && type==ANARI_GROUP) {
            group = *(ANARIGroup*)mem; // TODO: reference count
        } else if (strncmp(name,"transform",9)==0 && type==ANARI_FLOAT32_MAT3x4) {
            memcpy(transform,mem,sizeof(transform));
        } else {
            LOG(logging::Level::Warning) << "Instance: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


