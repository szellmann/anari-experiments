#include <string.h>
#include <type_traits>
#include "backend.hpp"
#include "light.hpp"
#include "logging.hpp"
#include "pointlight.hpp"
#include "quadlight.hpp"

namespace generic {

    Light::Light()
        : resourceHandle(new std::remove_pointer_t<ANARILight>)
    {
    }

    Light::~Light()
    {
    }

    ResourceHandle Light::getResourceHandle()
    {
        return resourceHandle;
    }

    void Light::commit()
    {
        backend::commit(*this);
    }

    void Light::release()
    {
    }

    void Light::retain()
    {
    }

    void Light::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"color",5)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(color,mem,sizeof(color));
        } else if (strncmp(name,"visible",7)==0 && type==ANARI_BOOL) {
            memcpy(&visible,mem,sizeof(visible));
        } else {
            LOG(logging::Level::Warning) << "Light: Unsupported parameter "
                << "/parameter type: " << name << " / " << type;
        }
    }

    void Light::unsetParameter(const char* name)
    {
        if (strncmp(name,"color",5)==0) {
            color[0] = color[1] = color[2] = 1.f;
        } else if (strncmp(name,"visible",7)==0) {
            visible = true;
        } else {
            LOG(logging::Level::Warning) << "Light: Unsupported parameter " << name;
        }
    }

    std::unique_ptr<Light> createLight(const char* subtype)
    {
        if (strncmp(subtype,"point",5)==0)
            return std::make_unique<PointLight>();
        else if (strncmp(subtype,"quad",4)==0)
            return std::make_unique<QuadLight>();
        else {
            LOG(logging::Level::Error) << "Light subtype unavailable: " << subtype;
            return std::make_unique<Light>();
        }
    }
} // generic


