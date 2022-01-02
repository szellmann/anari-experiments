#include <string.h>
#include <type_traits>
#include "logging.hpp"
#include "spatialfield.hpp"
#include "structuredregular.hpp"

namespace generic {

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

    void SpatialField::commit()
    {
        LOG(logging::Level::Warning) << "Calling commit on invalid spatial field\n";
    }

    void SpatialField::release()
    {
        LOG(logging::Level::Warning) << "Calling release on invalid spatial field\n";
    }

    void SpatialField::retain()
    {
        LOG(logging::Level::Warning) << "Calling retain on invalid spatial field\n";
    }

    void SpatialField::setParameter(const char* name,
                                    ANARIDataType type,
                                    const void* mem)
    {
        LOG(logging::Level::Warning) << "Setting parameter \"" << name
            << "\" on invalid spatial field!";
    }

    void SpatialField::unsetParameter(const char* name)
    {
        LOG(logging::Level::Warning) << "Unsetting parameter \"" << name
            << "\" on invalid spatial field!";
    }

    std::unique_ptr<SpatialField> createSpatialField(const char* subtype)
    {
        if (strncmp(subtype,"structuredRegular",17)==0)
            return std::make_unique<StructuredRegular>();
        else {
            LOG(logging::Level::Error) << "SpatialField subtype unavailable: " << subtype;
            return std::make_unique<SpatialField>();
        }
    }
} // generic


