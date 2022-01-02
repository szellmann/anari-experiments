#include <string.h>
#include <type_traits>
#include "backend.hpp"
#include "camera.hpp"
#include "logging.hpp"
#include "perspectivecamera.hpp"

namespace generic {

    Camera::Camera()
        : resourceHandle(new std::remove_pointer_t<ANARICamera>)
    {
    }

    Camera::~Camera()
    {
    }

    ResourceHandle Camera::getResourceHandle()
    {
        return resourceHandle;
    }

    void Camera::commit()
    {
        backend::commit(*this);
    }

    void Camera::release()
    {
    }

    void Camera::retain()
    {
    }

    void Camera::setParameter(const char* name,
                              ANARIDataType type,
                              const void* mem)
    {
        if (strncmp(name,"position",8)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(position,mem,sizeof(position));
        } else if (strncmp(name,"direction",9)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(direction,mem,sizeof(direction));
        } else if (strncmp(name,"up",2)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(up,mem,sizeof(up));
        } else if (strncmp(name,"transform",9)==0 && type==ANARI_FLOAT32_MAT3x4) {
            memcpy(transform,mem,sizeof(transform));
        } else if (strncmp(name,"imageRegion",11)==0 && type==ANARI_FLOAT32_BOX2) {
            memcpy(imageRegion,mem,sizeof(imageRegion));
        } else if (strncmp(name,"apertureRadius",14)==0 && type==ANARI_FLOAT32) {
            memcpy(&apertureRadius,mem,sizeof(apertureRadius));
        } else if (strncmp(name,"focusDistance",13)==0 && type==ANARI_FLOAT32) {
            memcpy(&focusDistance,mem,sizeof(focusDistance));
        } else if (strncmp(name,"stereoMode",11)==0 && type==ANARI_STRING) {
            stereoMode = (const char*)mem;
        } else if (strncmp(name,"interpupillaryDistance",23)==0 && type==ANARI_FLOAT32) {
            memcpy(&interpupillaryDistance,mem,sizeof(interpupillaryDistance));
        } else {
            LOG(logging::Level::Warning) << "Camera: Unsupported parameter "
                << "/parameter type: " << name << " / " << type;
        }
    }

    void Camera::unsetParameter(const char* name)
    {
        if (strncmp(name,"position",8)==0) {
            position[0] = position[1] = position[2] = 0.f;
        } else if (strncmp(name,"direction",9)==0) {
            direction[0] = direction[1] = 0.f; direction[2] = -1.f;
        } else if (strncmp(name,"up",2)==0) {
            up[0] = 0.f; up[1] = 1.f; up[2] = 0.f;
        } else if (strncmp(name,"transform",9)==0) {
            for (int i=0; i<3; ++i) {
                for (int j=0; j<3; ++j) {
                    transform[i][j] = (i==j) ? 1.f : 0.f;
                }
            }
            transform[3][0] = transform[3][1] = transform[3][2] = 0.f;
        } else if (strncmp(name,"imageRegion",11)==0) {
            imageRegion[0][0] = imageRegion[0][1] = 0.f;
            imageRegion[1][0] = imageRegion[1][1] = 1.f;
        } else if (strncmp(name,"apertureRadius",14)==0) {
            apertureRadius = 0.f;
        } else if (strncmp(name,"focusDistance",13)==0) {
            focusDistance = 1.f;
        } else if (strncmp(name,"stereoMode",11)==0) {
            stereoMode = "none";
        } else if (strncmp(name,"interpupillaryDistance",23)==0) {
            interpupillaryDistance = .0635f;
        } else {
            LOG(logging::Level::Warning) << "Camera: Unsupported parameter " << name;
        }
    }

    std::unique_ptr<Camera> createCamera(const char* subtype)
    {
        if (strncmp(subtype,"perspective",11)==0)
            return std::make_unique<PerspectiveCamera>();
        else {
            LOG(logging::Level::Error) << "Camera subtype unavailable: " << subtype;
            return std::make_unique<Camera>();
        }
    }
} // generic


