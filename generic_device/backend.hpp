#pragma once

#include "ao.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "instance.hpp"
#include "light.hpp"
#include "matte.hpp"
#include "object.hpp"
#include "pathtracer.hpp"
#include "perspectivecamera.hpp"
#include "pointlight.hpp"
#include "structuredregular.hpp"
#include "surface.hpp"
#include "trianglegeom.hpp"
#include "volume.hpp"
#include "world.hpp"

namespace generic {
    namespace backend {

        // Default implementation, generates an error message
        void commit(generic::Object& obj);

        void commit(generic::World& world);

        void commit(generic::Camera& cam);

        void commit(generic::PerspectiveCamera& cam);

        void commit(generic::Light& light);

        void commit(generic::PointLight& light);

        void commit(generic::Frame& frame);

        void commit(generic::TriangleGeom& geom);

        void commit(generic::Matte& mat);

        void commit(generic::Surface& surf);

        void commit(generic::Instance& inst);

        void commit(generic::StructuredRegular& sr);

        void commit(generic::Volume& vol);

        void commit(generic::Renderer& rend);

        void commit(generic::AO& ao);

        void commit(generic::Pathtracer& pt);

        void* map(generic::Frame& frame);
        void renderFrame(generic::Frame& frame);
        int wait(generic::Frame& frame, ANARIWaitMask m);

    } // backend
} // generic


