#include "array.hpp"
#include "backend.hpp"
#include "logging.hpp"
#include "structuredregular.hpp"

namespace generic {

    StructuredRegular::StructuredRegular()
        : SpatialField()
    {
    }

    StructuredRegular::~StructuredRegular()
    {
    }

    void StructuredRegular::commit()
    {
        if (data == nullptr)
            LOG(logging::Level::Error) << "SpatialField.StructuredRegular error: "
                << "data not set but is a required parameter";
        else
            backend::commit(*this);
    }

    void StructuredRegular::setParameter(const char* name,
                                         ANARIDataType type,
                                         const void* mem)
    {
        if (strncmp(name,"data",4)==0 && type==ANARI_ARRAY3D) {
            ANARIArray3D* handle = (ANARIArray3D*)mem;
            Array3D* arr = (Array3D*)GetResource(*handle);
            data = arr->data;
        } else if (strncmp(name,"filter",6)==0 && type==ANARI_STRING) {
            filter = (const char*)mem;
        } else {
            LOG(logging::Level::Warning) << "StructuredRegular: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


