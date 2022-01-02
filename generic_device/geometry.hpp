#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Geometry : public Object
    {
    public:
        Geometry();
        virtual ~Geometry();

        ResourceHandle getResourceHandle();

        virtual void commit();

        virtual void release();

        virtual void retain();

        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

        virtual void unsetParameter(const char* name);

    private:
        ANARIGeometry resourceHandle;

    };

    std::unique_ptr<Geometry> createGeometry(const char* subtype);

} // generic


