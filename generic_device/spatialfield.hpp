#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class SpatialField : public Object
    {
    public:
        SpatialField();
        virtual ~SpatialField();

        ResourceHandle getResourceHandle();

        // virtual, _if_ this is called it means something went wrong
        // (e.g., the user specified a wrong subtype)
        virtual void commit();

        virtual void release();

        virtual void retain();

        // virtual, _if_ this is called it means something went wrong
        // (e.g., the user specified a wrong subtype)
        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

    private:
        ANARISpatialField resourceHandle;
    };

    std::unique_ptr<SpatialField> createSpatialField(const char* subtype);

} // generic


