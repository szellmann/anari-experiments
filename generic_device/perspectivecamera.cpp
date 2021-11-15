#include <string.h>
#include "backend.hpp"
#include "perspectivecamera.hpp"

namespace generic {

    PerspectiveCamera::PerspectiveCamera()
        : Camera()
    {
    }

    PerspectiveCamera::~PerspectiveCamera()
    {
    }

    void PerspectiveCamera::commit()
    {
        backend::commit(*this);
        Camera::commit();
    }

    void PerspectiveCamera::release()
    {
        Camera::release();
    }

    void PerspectiveCamera::retain()
    {
        Camera::retain();
    }

    void PerspectiveCamera::setParameter(const char* name,
                                         ANARIDataType type,
                                         const void* mem)
    {
        if (strncmp(name,"fovy",4)==0 && type==ANARI_FLOAT32) {
            memcpy(&fovy,mem,sizeof(fovy));
        } else if (strncmp(name,"aspect",6)==0 && type==ANARI_FLOAT32) {
            memcpy(&aspect,mem,sizeof(aspect));
        } else {
            Camera::setParameter(name,type,mem);
        }
    }

} // generic


