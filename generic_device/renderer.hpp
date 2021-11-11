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

        constexpr static const char* Subtypes[3] = {
            "pathtracer", // 1st one is chosen by "default" 
            "ao",
            nullptr,     // last one most be NULL
        };

        constexpr static ANARIParameter Parameters[2] = {
            {"backgroundColor", ANARI_FLOAT32_VEC4},
            {nullptr, ANARI_UNKNOWN},
        };

    private:
        ANARIRenderer resourceHandle;
    };

    std::unique_ptr<Renderer> createRenderer(const char* subtype);

} // generic


