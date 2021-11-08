#pragma once

#include <anari/anari.h>
#include "resource.hpp"

namespace generic {

    struct Object : Resource
    {
        virtual ~Object() {}

        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem) = 0;
    };
} // generic


