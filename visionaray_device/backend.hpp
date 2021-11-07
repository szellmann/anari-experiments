#pragma once

#include "object.hpp"
#include "structuredregular.hpp"

namespace visionaray {
    namespace backend {

        // Default implementation, generates an error message
        void commit(Object& obj);


        void commit(StructuredRegular& sr);

    } // backend
} // visionaray


