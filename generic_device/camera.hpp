#pragma once

#include "object.hpp"
#include "resource.hpp"

namespace generic {

    class Camera : public Object
    {
    public:
        Camera();
        virtual ~Camera();

        ResourceHandle getResourceHandle();

        // Commits camera params. Make sure to call from subclass!
        virtual void commit();

        virtual void release();

        virtual void retain();

        // Sets common camera params. Make sure to call this from subclass!
        virtual void setParameter(const char* name,
                                  ANARIDataType type,
                                  const void* mem);

        float position[3] = {0.f,0.f,0.f};
        float direction[3] = {0.f,0.f,-1.f};
        float up[3] = {0.f,1.f,0.f};
        float transform[4][3] = {{1.f,0.f,0.f},{0.f,1.f,0.f},{0.f,0.f,1.f},{0.f,0.f,0.f}};
        float imageRegion[2][2] = {{0.f,0.f},{1.f,1.f}};
        float apertureRadius = 0.f;
        float focusDistance = 1.f;
        const char* stereoMode = "none";
        float interpupillaryDistance = .0635f;

    private:
        ANARICamera resourceHandle;
    };

    std::unique_ptr<Camera> createCamera(const char* subtype);

} // generic


