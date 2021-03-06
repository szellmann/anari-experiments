#include <string.h>
#include "backend.hpp"
#include "pointlight.hpp"

namespace generic {

    PointLight::PointLight()
        : Light()
    {
    }

    PointLight::~PointLight()
    {
    }

    void PointLight::commit()
    {
        backend::commit(*this);
        Light::commit();
    }

    void PointLight::release()
    {
        Light::release();
    }

    void PointLight::retain()
    {
        Light::retain();
    }

    void PointLight::setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem)
    {
        if (strncmp(name,"position",8)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(position,mem,sizeof(position));
        } else if (strncmp(name,"intensity",9)==0 && type==ANARI_FLOAT32) {
            memcpy(&intensity,mem,sizeof(intensity));
            intensityWasSet = true;
        } else if (strncmp(name,"power",5)==0 && type==ANARI_FLOAT32) {
            memcpy(&power,mem,sizeof(power));
            powerWasSet = true;
        } else if (strncmp(name,"radius",6)==0 && type==ANARI_FLOAT32) {
            memcpy(&radius,mem,sizeof(radius));
        } else if (strncmp(name,"radiance",8)==0 && type==ANARI_FLOAT32) {
            memcpy(&radiance,mem,sizeof(radiance));
        } else {
            Light::setParameter(name,type,mem);
        }
    }

    void PointLight::unsetParameter(const char* name)
    {
        if (strncmp(name,"position",8)==0) {
            position[0] = position[1] = position[2] = 0.f;
        } else if (strncmp(name,"intensity",9)==0) {
            intensity = 1.f;
            intensityWasSet = false;
        } else if (strncmp(name,"power",5)==0) {
            power = 1.f;
            powerWasSet = false;
        } else if (strncmp(name,"radius",6)==0) {
            radius = 0.f;
        } else if (strncmp(name,"radiance",8)==0) {
            radiance = 1.f;
        } else {
            Light::unsetParameter(name);
        }
    }

} // generic


