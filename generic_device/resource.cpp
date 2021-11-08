#include <assert.h>
#include <memory>
#include <unordered_map>
#include <utility>
#include "resource.hpp"

namespace generic {

    static std::unordered_map<ResourceHandle,std::unique_ptr<Resource>> resources;

    ResourceHandle RegisterResource(std::unique_ptr<Resource> res)
    {
        ResourceHandle handle = res->getResourceHandle();
        auto it = resources.insert({handle,std::move(res)});
        return handle;
    }

    Resource* GetResource(ResourceHandle handle)
    {
        auto it = resources.find(handle);
        if (it == resources.end())
            return nullptr;

        return it->second.get();
    }
} // generic


