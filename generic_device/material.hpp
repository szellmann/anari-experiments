#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Material : public Object
    {
    public:
        Material();
        virtual ~Material();

        ResourceHandle getResourceHandle();

        virtual void commit();

        virtual void release();

        virtual void retain();

        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

    private:
        ANARIMaterial resourceHandle;
    };

    std::unique_ptr<Material> createMaterial(const char* subtype);

} // generic


