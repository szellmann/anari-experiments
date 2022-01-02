#include <string.h>
#include <type_traits>
#include "ao.hpp"
#include "backend.hpp"
#include "logging.hpp"
#include "renderer.hpp"
#include "pathtracer.hpp"

namespace generic {

    Renderer::Renderer()
        : resourceHandle(new std::remove_pointer_t<ANARIRenderer>)
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::renderFrame(Frame*)
    {
        LOG(logging::Level::Warning) << "Renderer: renderFrame() called on base class";
    }

    ResourceHandle Renderer::getResourceHandle()
    {
        return resourceHandle;
    }

    void Renderer::commit()
    {
        backend::commit(*this);
    }

    void Renderer::release()
    {
    }

    void Renderer::retain()
    {
    }

    void Renderer::setParameter(const char* name,
                                ANARIDataType type,
                                const void* mem)
    {
        if (strncmp(name,"backgroundColor",15)==0 && type==ANARI_FLOAT32_VEC4) {
            memcpy(backgroundColor,mem,sizeof(backgroundColor));
        } else {
            LOG(logging::Level::Warning) << "Renderer: Unsupported parameter "
                << "/parameter type: " << name << " / " << type;
        }
    }

    void Renderer::unsetParameter(const char* name)
    {
        if (strncmp(name,"backgroundColor",15)==0) {
            backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = backgroundColor[3] = 0.f;
        } else {
            LOG(logging::Level::Warning) << "Renderer: Unsupported parameter " << name;
        }
    }

    std::unique_ptr<Renderer> createRenderer(const char* subtype)
    {
        if (strncmp(subtype,"default",7)==0 || strncmp(subtype,"pathtracer",10)==0)
            return std::make_unique<Pathtracer>();
        else if (strncmp(subtype,"ao",2)==0) {
            return std::make_unique<AO>();
        } else {
            LOG(logging::Level::Warning) << "Renderer subtype unavailable: \"" << subtype
                << "\", defaulting to \"pathtracer\"";
            return std::make_unique<Pathtracer>();
        }
    }
} // generic


