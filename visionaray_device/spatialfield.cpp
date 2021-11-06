#include <string.h>
#include <type_traits>
#include "logging.hpp"
#include "spatialfield.hpp"
#include "structuredregular.hpp"

namespace visionaray {

    SpatialField::SpatialField()
        : resourceHandle(new std::remove_pointer_t<ANARISpatialField>)
    {
    }

    SpatialField::~SpatialField()
    {
    }

    ResourceHandle SpatialField::getResourceHandle()
    {
        return resourceHandle;
    }

    std::unique_ptr<SpatialField> createSpatialField(const char* subtype)
    {
        if (strncmp(subtype,"structuredRegular",17)==0)
            return std::make_unique<StructuredRegular>();
        else {
            LOG(logging::Level::Error) << "Renderer subtype unavailable: " << subtype
                << ", defaulting to \"pathtracer\"";
            return std::make_unique<SpatialField>();
        }
    }
} // visionaray


