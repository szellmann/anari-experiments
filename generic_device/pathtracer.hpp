#pragma once

#include "renderer.hpp"
#include "resource.hpp"

namespace generic {

    class Pathtracer : public Renderer
    {
    public:
        Pathtracer();
       ~Pathtracer();

        void renderFrame(Frame* frame);

        void commit();

        void release();

        void retain();
    };

} // generic


