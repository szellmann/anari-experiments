#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace visionaray {

    class Volume : public Object
    {
    public:
        Volume();
       ~Volume();

        ResourceHandle getResourceHandle();

        void commit();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

    private:
        ANARIVolume resourceHandle;
    };

    std::unique_ptr<Volume> createVolume(const char* subtype);

} // visionaray


