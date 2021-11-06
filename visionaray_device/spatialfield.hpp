#pragma once

#include <cstdint>
#include "object.hpp"
#include "resource.hpp"

namespace visionaray {

    class SpatialField : public Object
    {
    public:
        SpatialField();
        virtual ~SpatialField();

        ResourceHandle getResourceHandle();

    private:
        ANARISpatialField resourceHandle;
    };

    std::unique_ptr<SpatialField> createSpatialField(const char* subtype);

} // visionaray


