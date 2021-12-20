#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Light : public Object
    {
    public:
        Light();
        virtual ~Light();

        ResourceHandle getResourceHandle();

        // Commits light params. Make sure to call from subclass!
        virtual void commit();

        virtual void release();

        virtual void retain();

        // Sets common light params. Make sure to call this from subclass!
        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

        float color[3] = {1.f,1.f,1.f};

    private:
        ANARILight resourceHandle;
    };

    std::unique_ptr<Light> createLight(const char* subtype);

} // generic


