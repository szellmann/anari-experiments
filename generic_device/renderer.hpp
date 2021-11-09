#pragma once

#include "frame.hpp"
#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Renderer : public Object
    {
    public:
        Renderer();
        virtual ~Renderer();

        virtual void renderFrame(Frame* frame);

        ResourceHandle getResourceHandle();

        virtual void commit();

        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

        float backgroundColor[4] = {0.f,0.f,0.f,0.f};
    private:
        ANARIRenderer resourceHandle;
    };

    std::unique_ptr<Renderer> createRenderer(const char* subtype);

} // generic


