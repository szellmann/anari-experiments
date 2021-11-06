#include <assert.h>
#include <memory>
#include <unordered_map>
#include <utility>
#include "object.hpp"
#include "resource.hpp"

namespace visionaray {

    static std::unordered_map<ResourceHandle,std::unique_ptr<Object>> resources;

    ResourceHandle RegisterResource(std::unique_ptr<Object> res)
    {
        ResourceHandle handle = res->getResourceHandle();
        auto it = resources.insert({handle,std::move(res)});
        return handle;
    }

    const Object* GetResource(ResourceHandle handle)
    {
        auto it = resources.find(handle);
        if (it == resources.end())
            return nullptr;

        return it->second.get();
    }
} // visionaray


