#include "volume.hpp"

namespace visionaray {

    Volume::Volume()
        : resourceHandle(new std::remove_pointer_t<ANARIVolume>)
    {
    }

    Volume::~Volume()
    {
    }

    ResourceHandle Volume::getResourceHandle()
    {
        return resourceHandle;
    }

    void Volume::commit()
    {
    }

    void Volume::setParameter(const char* name,
                              ANARIDataType type,
                              const void* mem)
    {
    }

    std::unique_ptr<Volume> createVolume(const char* subtype)
    {
        return std::make_unique<Volume>();
    }

} // visionaray


