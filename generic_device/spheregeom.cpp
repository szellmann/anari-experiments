#include <string.h>
#include "backend.hpp"
#include "spheregeom.hpp"
#include "logging.hpp"

namespace generic {

    SphereGeom::SphereGeom()
    {
    }

    SphereGeom::~SphereGeom()
    {
    }

    void SphereGeom::commit()
    {
        if (vertex_position == nullptr)
            LOG(logging::Level::Error) << "Sphere error: vertex.position not set but "
                << "is a required parameter";
        else
            backend::commit(*this);
    }

    void SphereGeom::release()
    {
    }

    void SphereGeom::retain()
    {
    }

    void SphereGeom::setParameter(const char* name,
                                    ANARIDataType type,
                                    const void* mem)
    {
        if (strncmp(name,"vertex.position",15)==0 && type==ANARI_ARRAY1D) {
            vertex_position = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.color",12)==0 && type==ANARI_ARRAY1D) {
            vertex_color = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.attribute0",17)==0 && type==ANARI_ARRAY1D) {
            vertex_attribute0 = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.attribute1",17)==0 && type==ANARI_ARRAY1D) {
            vertex_attribute1 = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.attribute2",17)==0 && type==ANARI_ARRAY1D) {
            vertex_attribute2 = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.attribute3",17)==0 && type==ANARI_ARRAY1D) {
            vertex_attribute3 = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"primitive.index",15)==0 && type==ANARI_ARRAY1D) {
            primitive_index = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"primitive.radius",16)==0 && type==ANARI_ARRAY1D) {
            primitive_radius = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.radius",13)==0 && type==ANARI_ARRAY1D) {
            vertex_radius = *(ANARIArray1D*)mem; // TODO: reference count
        } 
        else if (strncmp(name,"radius",6)==0 && type==ANARI_FLOAT32) {
            memcpy(&radius,mem,sizeof(radius));
        } else {
            LOG(logging::Level::Warning) << "Sphere: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void SphereGeom::unsetParameter(const char* name)
    {
        if (strncmp(name,"vertex.position",15)==0) {
            vertex_position = nullptr;
        } else if (strncmp(name,"vertex.color",12)==0) {
            vertex_color = nullptr;
        } else if (strncmp(name,"vertex.attribute0",17)==0) {
            vertex_attribute0 = nullptr;
        } else if (strncmp(name,"vertex.attribute1",17)==0) {
            vertex_attribute1 = nullptr;
        } else if (strncmp(name,"vertex.attribute2",17)==0) {
            vertex_attribute2 = nullptr;
        } else if (strncmp(name,"vertex.attribute3",17)==0) {
            vertex_attribute3 = nullptr;
        } else if (strncmp(name,"primitive.index",15)==0) {
            primitive_index = nullptr;
        } else if (strncmp(name,"primitive.radius",16)==0) {
            primitive_radius = nullptr;
        } else if (strncmp(name,"radius",6)==0) {
            radius = 1.f;
        } else {
            LOG(logging::Level::Warning) << "Sphere: Unsupported parameter " << name;
        }
    }

} // generic


