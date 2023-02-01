
#pragma once

#include <anari/backend/utilities/ParameterizedObject.h>
#include <anari/backend/DeviceImpl.h>

namespace generic {

    struct Device : anari::DeviceImpl
                  , anari::ParameterizedObject
    {
        //--- Data Arrays ---------------------------------

        ANARIArray1D newArray1D(const void* appMemory,
                                ANARIMemoryDeleter deleter,
                                const void* userdata,
                                ANARIDataType,
                                uint64_t numItems1,
                                uint64_t byteStride1) override;

        ANARIArray2D newArray2D(const void* appMemory,
                                ANARIMemoryDeleter deleter,
                                const void* userdata,
                                ANARIDataType,
                                uint64_t numItems1,
                                uint64_t numItems2,
                                uint64_t byteStride1,
                                uint64_t byteStride2) override;

        ANARIArray3D newArray3D(const void* appMemory,
                                ANARIMemoryDeleter deleter,
                                const void* userdata,
                                ANARIDataType,
                                uint64_t numItems1,
                                uint64_t numItems2,
                                uint64_t numItems3,
                                uint64_t byteStride1,
                                uint64_t byteStride2,
                                uint64_t byteStride3) override;

        void* mapArray(ANARIArray) override;

        void unmapArray(ANARIArray) override;

        //--- Renderable Objects --------------------------

        ANARILight newLight(const char* type) override;

        ANARICamera newCamera(const char* type) override;

        ANARIGeometry newGeometry(const char* type) override;

        ANARISpatialField newSpatialField(const char* type) override;

        ANARISurface newSurface() override;

        ANARIVolume newVolume(const char* type) override;

        //--- Surface Meta-Data ---------------------------

        ANARIMaterial newMaterial(const char* material_type) override;

        ANARISampler newSampler(const char* type) override;

        //--- Instancing ----------------------------------

        ANARIGroup newGroup() override;

        ANARIInstance newInstance() override;

        //--- Top-level Worlds ----------------------------

        ANARIWorld newWorld() override;

        //--- Object + Parameter Lifetime Management ------

        void setParameter(ANARIObject object,
                          const char* name,
                          ANARIDataType type,
                          const void* mem) override;

        void unsetParameter(ANARIObject object,
                            const char* name) override;

        void commitParameters(ANARIObject object) override;

        void release(ANARIObject _obj) override;

        void retain(ANARIObject _obj) override;

        //--- Object Query Interface ----------------------

        int getProperty(ANARIObject object,
                        const char* name,
                        ANARIDataType type,
                        void* mem,
                        uint64_t size,
                        ANARIWaitMask mask) override;

        //--- FrameBuffer Manipulation --------------------

        ANARIFrame newFrame() override;

        const void* frameBufferMap(ANARIFrame fb,
                                   const char* channel,
                                   uint32_t* width,
                                   uint32_t* height,
                                   ANARIDataType* pixelType) override;

        void frameBufferUnmap(ANARIFrame fb,
                              const char* channel) override;

        //--- Frame Rendering -----------------------------

        ANARIRenderer newRenderer(const char* type) override;

        void renderFrame(ANARIFrame) override;

        int frameReady(ANARIFrame,
                       ANARIWaitMask) override;

        void discardFrame(ANARIFrame) override;

        //--- Extension interface -------------------------

        //ANARIObject newObject(const char* objectType,
        //                      const char* type);

        //void (*getProcAddress(const char *name))(void);

    public:
        Device();
       ~Device();
    };

} // generic


