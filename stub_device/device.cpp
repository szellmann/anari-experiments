#include <anari/detail/Library.h>

#include "device.hpp"

namespace stub {

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
        return nullptr;
    }

    const void* Device::frameBufferMap(ANARIFrame fb,
                               const char* channel)
    {
        return nullptr;
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
} // stub


static char deviceName[] = "stub";

ANARI_DEFINE_LIBRARY_INIT(stub)
{
  printf("...loaded stub library!\n");
  anari::Device::registerType<stub::Device>(deviceName);
}

ANARI_DEFINE_LIBRARY_GET_DEVICE_SUBTYPES(stub, libdata)
{
  static const char *devices[] = {deviceName, nullptr};
  return devices;
}
ANARI_DEFINE_LIBRARY_GET_OBJECT_SUBTYPES(
    stub, libdata, deviceSubtype, objectType)
{
  if (objectType == ANARI_RENDERER) {
    static std::vector<const char *> renderers;
    renderers.clear();
    //anari::example_device::Renderer::init();
    //for (auto &r : *anari::example_device::Renderer::g_renderers)
    //  renderers.push_back(r.first.c_str());
    //renderers.push_back(nullptr);
    return renderers.data();
  }
  return nullptr;
}
ANARI_DEFINE_LIBRARY_GET_OBJECT_PARAMETERS(
    stub, libdata, deviceSubtype, objectSubtype, objectType)
{
  //if (objectType == ANARI_RENDERER)
  //  return anari::example_device::Renderer::g_parameters;
  return nullptr;
}
ANARI_DEFINE_LIBRARY_GET_PARAMETER_PROPERTY(stub,
    libdata,
    deviceSubtype,
    objectSubtype,
    objectType,
    parameterName,
    parameterType,
    propertyName,
    propertyType)
{
  //if (objectType == ANARI_RENDERER) {
  //  int i = 0;
  //  while (anari::example_device::Renderer::g_parameters[i].name != nullptr) {
  //    if (std::string(anari::example_device::Renderer::g_parameters[i].name)
  //            == std::string(parameterName)
  //        && (anari::example_device::Renderer::g_parameters[i].type
  //            == parameterType))
  //      break;

  //    i++;
  //  }
  //  if (anari::example_device::Renderer::g_parameters[i].name == nullptr)
  //    return nullptr;
  //  if (propertyType == ANARI_STRING
  //      && std::string(propertyName) == std::string("description"))
  //    return anari::example_device::Renderer::g_parameterinfos[i].desc;
  //  if (propertyType == ANARI_BOOL
  //      && std::string(propertyName) == std::string("required"))
  //    return &anari::example_device::Renderer::g_parameterinfos[i].req;
  //  if (propertyType == parameterType
  //      && std::string(propertyName) == std::string("default"))
  //    return &anari::example_device::Renderer::g_parameterinfos[i].def;
  //}
  return nullptr;
}

extern "C" ANARIDevice anariNewExampleDevice()
{
    return (ANARIDevice) new stub::Device;
}
