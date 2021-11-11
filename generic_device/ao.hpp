#pragma once

#include "renderer.hpp"
#include "resource.hpp"

namespace generic {

    class AO : public Renderer
    {
    public:
        AO();
       ~AO();

        void renderFrame(Frame* frame);

        void commit();
    };

} // generic


