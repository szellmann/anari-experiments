#include "ao.hpp"
#include "backend.hpp"

namespace generic {

    AO::AO()
        : Renderer()
    {
    }

    AO::~AO()
    {
    }

    void AO::renderFrame(Frame* frame)
    {
        backend::renderFrame(*frame);
    }

    void AO::commit()
    {
        backend::commit(*this);

        Renderer::commit();
    }
} // generic


