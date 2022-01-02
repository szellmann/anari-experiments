#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Instance : public Object
    {
    public:
        Instance();
       ~Instance();

        ResourceHandle getResourceHandle();

        void commit();

        void release();

        void retain();

        void setParameter(const char* name,
                          ANARIDataType type,
                          const void* mem);

        void unsetParameter(const char* name);

        ANARIGroup group = nullptr;
        float transform[4][3] = {{1.f,0.f,0.f},{0.f,1.f,0.f},{0.f,0.f,1.f},{0.f,0.f,0.f}};

    private:
        ANARIInstance resourceHandle = nullptr;
    };

} // generic


