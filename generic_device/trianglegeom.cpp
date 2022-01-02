#include <string.h>
#include "backend.hpp"
#include "logging.hpp"
#include "trianglegeom.hpp"

namespace generic {

    TriangleGeom::TriangleGeom()
    {
    }

    TriangleGeom::~TriangleGeom()
    {
    }

    void TriangleGeom::commit()
    {
        if (vertex_position == nullptr)
            LOG(logging::Level::Error) << "Triangle error: vertex.position not set but "
                << "is a required parameter";
        else
            backend::commit(*this);
    }

    void TriangleGeom::release()
    {
    }

    void TriangleGeom::retain()
    {
    }

    void TriangleGeom::setParameter(const char* name,
                                    ANARIDataType type,
                                    const void* mem)
    {
        if (strncmp(name,"vertex.position",15)==0 && type==ANARI_ARRAY1D) {
            vertex_position = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"vertex.normal",13)==0 && type==ANARI_ARRAY1D) {
            vertex_normal = *(ANARIArray1D*)mem; // TODO: reference count
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
        } else {
            LOG(logging::Level::Warning) << "Triangle: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void TriangleGeom::unsetParameter(const char* name)
    {
        if (strncmp(name,"vertex.position",15)==0) {
            vertex_position = nullptr;
        } else if (strncmp(name,"vertex.normal",13)==0) {
            vertex_normal = nullptr;
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
        } else {
            LOG(logging::Level::Warning) << "Triangle: Unsupported parameter " << name;
        }
    }

} // generic


