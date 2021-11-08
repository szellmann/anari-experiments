#include <string.h>
#include <type_traits>
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

    void Renderer::setParameter(const char* name,
                                ANARIDataType type,
                                const void* mem)
    {
    }

    std::unique_ptr<Renderer> createRenderer(const char* subtype)
    {
        if (strncmp(subtype,"pathtracer",10)==0)
            return std::make_unique<Pathtracer>();
        else {
            LOG(logging::Level::Warning) << "Renderer subtype unavailable: " << subtype
                << ", defaulting to \"pathtracer\"";
            return std::make_unique<Pathtracer>();
        }
    }
} // generic


