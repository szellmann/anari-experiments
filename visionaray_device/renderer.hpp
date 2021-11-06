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

    private:
        ANARIRenderer resourceHandle;
    };

    std::unique_ptr<Renderer> createRenderer(const char* subtype);

} // visionaray


