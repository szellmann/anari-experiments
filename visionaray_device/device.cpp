#include <anari/detail/Library.h>

#include "device.hpp"
#include "frame.hpp"

namespace visionaray {

    //--- Device API --------------------------------------

    int Device::deviceImplements(const char* extensions)
    {
        return 0;
    }

    void Device::deviceSetParameter(const char* id,
                                    ANARIDataType type,
                                    const void* mem)
    {
    }

    void Device::deviceUnsetParameter(const char* id)
    {
    }

    void Device::deviceCommit()
    {
    }

    void Device::deviceRetain()
    {
    }

    void Device::deviceRelease()
    {
    }

    //--- Data Arrays -------------------------------------

    ANARIArray1D Device::newArray1D(void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    void* userdata,
                                    ANARIDataType,
                                    uint64_t numItems1,
                                    uint64_t byteStride1)
    {
        return nullptr;
    }

    ANARIArray2D Device::newArray2D(void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    void* userdata,
                                    ANARIDataType,
                                    uint64_t numItems1,
                                    uint64_t numItems2,
                                    uint64_t byteStride1,
                                    uint64_t byteStride2)
    {
        return nullptr;
    }

    ANARIArray3D Device::newArray3D(void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    void* userdata,
                                    ANARIDataType,
                                    uint64_t numItems1,
                                    uint64_t numItems2,
                                    uint64_t numItems3,
                                    uint64_t byteStride1,
                                    uint64_t byteStride2,
                                    uint64_t byteStride3)
    {
        return nullptr;
    }

    void* Device::mapArray(ANARIArray)
    {
        return nullptr;
    }

    void Device::unmapArray(ANARIArray)
    {
    }

    //--- Renderable Objects ------------------------------

    ANARILight Device::newLight(const char* type)
    {
        return nullptr;
    }

    ANARICamera Device::newCamera(const char* type)
    {
        return nullptr;
    }

    ANARIGeometry Device::newGeometry(const char* type)
    {
        return nullptr;
    }

    ANARISpatialField Device::newSpatialField(const char* type)
    {
        return nullptr;
    }

    ANARISurface Device::newSurface()
    {
        return nullptr;
    }

    ANARIVolume Device::newVolume(const char* type)
    {
        return nullptr;
    }

    //--- Surface Meta-Data -------------------------------

    ANARIMaterial Device::newMaterial(const char* material_type)
    {
        return nullptr;
    }

    ANARISampler Device::newSampler(const char* type)
    {
        return nullptr;
    }

    //--- Instancing --------------------------------------

    ANARIGroup Device::newGroup()
    {
        return nullptr;
    }

    ANARIInstance Device::newInstance()
    {
        return nullptr;
    }

    //--- Top-level Worlds --------------------------------

    ANARIWorld Device::newWorld()
    {
        return nullptr;
    }

    //--- Object + Parameter Lifetime Management ----------

    void Device::setParameter(ANARIObject object,
                              const char* name,
                              ANARIDataType type,
                              const void* mem)
    {
    }

    void Device::unsetParameter(ANARIObject object,
                                const char* name)
    {
    }

    void Device::commit(ANARIObject object)
    {
    }

    void Device::release(ANARIObject _obj) 
    {
    }

    void Device::retain(ANARIObject _obj)
    {
    }

    //--- Object Query Interface --------------------------

    int Device::getProperty(ANARIObject object,
                            const char* name,
                            ANARIDataType type,
                            void* mem,
                            uint64_t size,
                            ANARIWaitMask mask)
    {
        return 0;
    }

    //--- FrameBuffer Manipulation ------------------------

    ANARIFrame Device::newFrame()
    {
        return (ANARIFrame)RegisterResource(std::make_unique<Frame>());
    }

    const void* Device::frameBufferMap(ANARIFrame fb,
                                       const char* channel)
    {
        Frame* frame = (Frame*)GetResource(fb);
        return frame->map(channel);
    }

    void Device::frameBufferUnmap(ANARIFrame fb,
                                  const char* channel)
    {
    }

    //--- Frame Rendering ---------------------------------

    ANARIRenderer Device::newRenderer(const char* type)
    {
        return nullptr;
    }

    void Device::renderFrame(ANARIFrame)
    {
    }

    int Device::frameReady(ANARIFrame,
                           ANARIWaitMask)
    {
        return 0;
    }

    void Device::discardFrame(ANARIFrame)
    {
    }

    //--- Extension inteface ------------------------------

    //ANARIObject Devcie::newObject(const char* objectType,
    //                              const char* type);

    //void (*getProcAddress(const char *name))(void);

    Device::Device()
    {

    }

    Device::~Device()
    {

    }
} // visionaray


static char deviceName[] = "visionaray";

ANARI_DEFINE_LIBRARY_INIT(visionaray)
{
  printf("...loaded visionaray library!\n");
  anari::Device::registerType<visionaray::Device>(deviceName);
}

ANARI_DEFINE_LIBRARY_GET_DEVICE_SUBTYPES(visionaray, libdata)
{
  static const char *devices[] = {deviceName, nullptr};
  return devices;
}
ANARI_DEFINE_LIBRARY_GET_OBJECT_SUBTYPES(
    visionaray, libdata, deviceSubtype, objectType)
{
  if (objectType == ANARI_RENDERER) {
    static const char* renderers[] = {
        "pathtracer", // 1st one is chosen by "default" 
        nullptr,     // last one most be NULL
    };
    return renderers;
  }
  return nullptr;
}
ANARI_DEFINE_LIBRARY_GET_OBJECT_PARAMETERS(
    visionaray, libdata, deviceSubtype, objectSubtype, objectType)
{
  return nullptr;
}
ANARI_DEFINE_LIBRARY_GET_PARAMETER_PROPERTY(visionaray,
    libdata,
    deviceSubtype,
    objectSubtype,
    objectType,
    parameterName,
    parameterType,
    propertyName,
    propertyType)
{
  return nullptr;
}

extern "C" ANARIDevice anariNewExampleDevice()
{
    return (ANARIDevice) new visionaray::Device;
}
