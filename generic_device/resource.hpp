#pragma once

#include <memory>

namespace generic {

    typedef void *ResourceHandle;

    struct Resource {
        virtual ~Resource() {}
        virtual ResourceHandle getResourceHandle() = 0;

        virtual void commit() = 0;
    };

    ResourceHandle RegisterResource(std::unique_ptr<Resource> res);
    Resource* GetResource(ResourceHandle handle);

} // generic


