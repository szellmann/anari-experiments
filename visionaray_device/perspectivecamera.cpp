#include <string.h>
#include "perspectivecamera.hpp"

namespace visionaray {

    PerspectiveCamera::PerspectiveCamera()
        : Camera()
    {
    }

    PerspectiveCamera::~PerspectiveCamera()
    {
    }

    void PerspectiveCamera::commit()
    {
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

} // visionaray


