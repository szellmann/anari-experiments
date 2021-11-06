#pragma once

#include <memory>

namespace visionaray {

    struct Object;

    typedef void *ResourceHandle;
    ResourceHandle RegisterResource(std::unique_ptr<Object> res);
    const Object* GetResource(ResourceHandle handle);

} // visionaray


