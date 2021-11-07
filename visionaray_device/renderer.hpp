#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace visionaray {

    class Renderer : public Object
    {
    public:
        Renderer();
        virtual ~Renderer();

        ResourceHandle getResourceHandle();

        // Usually a noop for renderers (subtypes _may_ still override)
        virtual void commit() {}

        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

    private:
        ANARIRenderer resourceHandle;
    };

    std::unique_ptr<Renderer> createRenderer(const char* subtype);

} // visionaray


