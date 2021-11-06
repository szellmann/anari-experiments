#include <string.h>
#include <type_traits>
#include "logging.hpp"
#include "renderer.hpp"
#include "pathtracer.hpp"

namespace visionaray {

    Renderer::Renderer()
        : resourceHandle(new std::remove_pointer_t<ANARIRenderer>)
    {
    }

    Renderer::~Renderer()
    {
    }

    ResourceHandle Renderer::getResourceHandle()
    {
        return resourceHandle;
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
} // visionaray


