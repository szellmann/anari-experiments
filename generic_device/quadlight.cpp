#include <string.h>
#include "backend.hpp"
#include "quadlight.hpp"

namespace generic {

    QuadLight::QuadLight()
        : Light()
    {
    }

    QuadLight::~QuadLight()
    {
    }

    void QuadLight::commit()
    {
        backend::commit(*this);
        Light::commit();
    }

    void QuadLight::release()
    {
        Light::release();
    }

    void QuadLight::retain()
    {
        Light::retain();
    }

    void QuadLight::setParameter(const char* name,
                                 ANARIDataType type,
                                 const void* mem)
    {
        if (strncmp(name,"position",8)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(position,mem,sizeof(position));
        } else if (strncmp(name,"edge1",5)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(edge1,mem,sizeof(edge1));
        } else if (strncmp(name,"edge2",5)==0 && type==ANARI_FLOAT32_VEC3) {
            memcpy(edge2,mem,sizeof(edge2));
        } else if (strncmp(name,"intensity",9)==0 && type==ANARI_FLOAT32) {
            memcpy(&intensity,mem,sizeof(intensity));
            intensityWasSet = true;
        } else if (strncmp(name,"power",5)==0 && type==ANARI_FLOAT32) {
            memcpy(&power,mem,sizeof(power));
            powerWasSet = true;
        } else if (strncmp(name,"radiance",8)==0 && type==ANARI_FLOAT32) {
            memcpy(&radiance,mem,sizeof(radiance));
        } else if (strncmp(name,"side",4)==0 && type==ANARI_STRING) {
            const char* src = (const char*)mem;
            strncpy(side,src,std::min(sizeof(side),strlen(src)));
        } else {
            Light::setParameter(name,type,mem);
        }
    }

    void QuadLight::unsetParameter(const char* name)
    {
        if (strncmp(name,"position",8)==0) {
            position[0] = position[1] = position[2] = 0.f;
        } else if (strncmp(name,"edge1",5)==0) {
            edge1[0] = 1.f; edge1[1] = edge1[2] = 0.f;
        }  else if (strncmp(name,"edge2",5)==0) {
            edge2[1] = 1.f; edge2[0] = edge2[2] = 0.f;
        } else if (strncmp(name,"intensity",9)==0) {
            intensity = 1.f;
            intensityWasSet = false;
        } else if (strncmp(name,"power",5)==0) {
            power = 1.f;
            powerWasSet = false;
        } else if (strncmp(name,"radiance",8)==0) {
            radiance = 1.f;
        } else if (strncmp(name,"side",4)==0) {
            const char* src = "front";
            strncpy(side,src,strlen(src));
        } else {
            Light::unsetParameter(name);
        }
    }

} // generic


