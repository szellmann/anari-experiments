#include "backend.hpp"
#include "pathtracer.hpp"

namespace generic {

    Pathtracer::Pathtracer()
        : Renderer()
    {
    }

    Pathtracer::~Pathtracer()
    {
    }

    void Pathtracer::renderFrame(Frame* frame)
    {
        backend::renderFrame(*frame);
    }

    void Pathtracer::commit()
    {
        backend::commit(*this);

        Renderer::commit();
    }
} // generic


