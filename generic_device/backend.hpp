#pragma once

#include "camera.hpp"
#include "frame.hpp"
#include "object.hpp"
#include "pathtracer.hpp"
#include "perspectivecamera.hpp"
#include "structuredregular.hpp"
#include "world.hpp"

namespace generic {
    namespace backend {

        // Default implementation, generates an error message
        void commit(Object& obj);

        void commit(World& world);

        void commit(Camera& cam);

        void commit(PerspectiveCamera& cam);

        void commit(Frame& frame);

        void commit(StructuredRegular& sr);

        void commit(Volume& vol);

        void commit(Renderer& rend);

        void commit(Pathtracer& pt);

        void* map(Frame& frame);
        void renderFrame(Frame& frame);

    } // backend
} // generic


