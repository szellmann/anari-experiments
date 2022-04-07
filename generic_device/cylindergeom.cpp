#include <string.h>
#include "backend.hpp"
#include "cylindergeom.hpp"
#include "logging.hpp"

namespace generic {

    CylinderGeom::CylinderGeom()
    {
    }

    CylinderGeom::~CylinderGeom()
    {
    }

    void CylinderGeom::commit()
    {
        if (vertex_position == nullptr)
            LOG(logging::Level::Error) << "Cylinder error: vertex.position not set but "
                << "is a required parameter";
        else
            backend::commit(*this);
    }

    void CylinderGeom::release()
    {
    }

    void CylinderGeom::retain()
    {
    }

    void CylinderGeom::setParameter(const char* name,
                                    ANARIDataType type,
                                    const void* mem)
    {
        if (strncmp(name,"vertex.position",15)==0 && type==ANARI_ARRAY1D) {
            vertex_position = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.cap",10)==0 && type==ANARI_ARRAY1D) {
            vertex_cap = *(ANARIArray1D*)mem; // TODO: reference count
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
        } else if (strncmp(name,"radius",6)==0 && type==ANARI_FLOAT32) {
            memcpy(&radius,mem,sizeof(radius));
        } else if (strncmp(name,"caps",4)==0 && type==ANARI_STRING) {
            caps = (const char*)mem;
        } else {
            LOG(logging::Level::Warning) << "Cylinder: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void CylinderGeom::unsetParameter(const char* name)
    {
        if (strncmp(name,"vertex.position",15)==0) {
            vertex_position = nullptr;
        } else if (strncmp(name,"vertex.cap",10)==0) {
            vertex_cap = nullptr;
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
        } else if (strncmp(name,"caps",4)==0) {
            caps = "none";
        } else {
            LOG(logging::Level::Warning) << "Cylinder: Unsupported parameter " << name;
        }
    }

} // generic


