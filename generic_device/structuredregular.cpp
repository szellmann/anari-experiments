#include <string.h>
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

    void StructuredRegular::release()
    {
    }

    void StructuredRegular::retain()
    {
    }

    void StructuredRegular::setParameter(const char* name,
                                         ANARIDataType type,
                                         const void* mem)
    {
        if (strncmp(name,"data",4)==0 && type==ANARI_ARRAY3D) {
            data = *(ANARIArray3D*)mem;
        } else if (strncmp(name,"filter",6)==0 && type==ANARI_STRING) {
            filter = (const char*)mem;
        } else {
            LOG(logging::Level::Warning) << "StructuredRegular: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void StructuredRegular::unsetParameter(const char* name)
    {
        if (strncmp(name,"data",4)==0) {
            data = nullptr;
        } else if (strncmp(name,"filter",6)==0) {
            filter = "linear";
        } else {
            LOG(logging::Level::Warning) << "StructuredRegular: Unsupported parameter " << name;
        }
    }

} // generic


