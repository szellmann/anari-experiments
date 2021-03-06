#include <string.h>
#include <type_traits>
#include "backend.hpp"
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
        backend::commit(*this);
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

    void Instance::unsetParameter(const char* name)
    {
        if (strncmp(name,"group",5)==0) {
            group = nullptr;
        } else if (strncmp(name,"transform",9)==0) {
            for (int i=0; i<3; ++i) {
                for (int j=0; j<3; ++j) {
                    transform[i][j] = (i==j) ? 1.f : 0.f;
                }
            }
            transform[3][0] = transform[3][1] = transform[3][2] = 0.f;
        } else {
            LOG(logging::Level::Warning) << "Instance: Unsupported parameter " << name;
        }
    }

} // generic


