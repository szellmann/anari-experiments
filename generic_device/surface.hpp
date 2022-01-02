#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Surface : public Object
    {
    public:
        Surface();
       ~Surface();

        ResourceHandle getResourceHandle();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        ANARIGeometry geometry = nullptr;
        ANARIMaterial material = nullptr;

    private:
        ANARISurface resourceHandle = nullptr;
    };

} // generic


