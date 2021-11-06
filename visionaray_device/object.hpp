#pragma once

#include <anari/anari.h>
#include "resource.hpp"

namespace visionaray {

    struct Object
    {
        virtual ~Object() {}
        virtual ResourceHandle getResourceHandle() = 0;
    };
} // visionaray


