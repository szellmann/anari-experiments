#include <string.h>
#include "backend.hpp"
#include "logging.hpp"
#include "matte.hpp"

namespace generic {

    Matte::Matte()
        : Material()
    {
    }

    Matte::~Matte()
    {
    }

    void Matte::commit()
    {
        backend::commit(*this);
    }

    void Matte::release()
    {
    }

    void Matte::retain()
    {
    }

    void Matte::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"color",5)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(color,mem,sizeof(color));
        } else {
            LOG(logging::Level::Warning) << "matte: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void Matte::unsetParameter(const char* name)
    {
        if (strncmp(name,"color",5)==0) {
            color[0] = color[1] = color[2] = .8f;
        } else {
            LOG(logging::Level::Warning) << "matte: Unsupported parameter " << name;
        }
    }

} // generic


