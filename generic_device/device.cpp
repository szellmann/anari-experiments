#include <anari/backend/LibraryImpl.h>

#include "array.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "frame.hpp"
#include "geometry.hpp"
#include "group.hpp"
#include "instance.hpp"
#include "light.hpp"
#include "logging.hpp"
#include "material.hpp"
#include "renderer.hpp"
#include "spatialfield.hpp"
#include "surface.hpp"
#include "volume.hpp"
#include "world.hpp"

namespace generic {

    //--- Data Arrays -------------------------------------

    ANARIArray1D Device::newArray1D(const void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    const void* userdata,
                                    const ANARIDataType elementType,
                                    uint64_t numItems1,
                                    uint64_t byteStride1)
    {
        return (ANARIArray1D)RegisterResource(std::make_unique<Array1D>(
            (const uint8_t*)appMemory,deleter,userdata,elementType,numItems1,byteStride1));
    }

    ANARIArray2D Device::newArray2D(const void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    const void* userdata,
                                    ANARIDataType elementType,
                                    uint64_t numItems1,
                                    uint64_t numItems2,
                                    uint64_t byteStride1,
                                    uint64_t byteStride2)
    {
        return (ANARIArray2D)RegisterResource(std::make_unique<Array2D>(
            (const uint8_t*)appMemory,deleter,userdata,elementType,numItems1,numItems2,
            byteStride1,byteStride2));
    }

    ANARIArray3D Device::newArray3D(const void* appMemory,
                                    ANARIMemoryDeleter deleter,
                                    const void* userdata,
                                    ANARIDataType elementType,
                                    uint64_t numItems1,
                                    uint64_t numItems2,
                                    uint64_t numItems3,
                                    uint64_t byteStride1,
                                    uint64_t byteStride2,
                                    uint64_t byteStride3)
    {
        return (ANARIArray3D)RegisterResource(std::make_unique<Array3D>(
            (const uint8_t*)appMemory,deleter,userdata,elementType,numItems1,numItems2,
            numItems3,byteStride1,byteStride2,byteStride3));
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
        return (ANARILight)RegisterResource(createLight(type));
    }

    ANARICamera Device::newCamera(const char* type)
    {
        return (ANARICamera)RegisterResource(createCamera(type));
    }

    ANARIGeometry Device::newGeometry(const char* type)
    {
        return (ANARIGeometry)RegisterResource(createGeometry(type));
    }

    ANARISpatialField Device::newSpatialField(const char* type)
    {
        return (ANARISpatialField)RegisterResource(createSpatialField(type));
    }

    ANARISurface Device::newSurface()
    {
        return (ANARISurface)RegisterResource(std::make_unique<Surface>());
    }

    ANARIVolume Device::newVolume(const char* type)
    {
        return (ANARIVolume)RegisterResource(createVolume(type));
    }

    //--- Surface Meta-Data -------------------------------

    ANARIMaterial Device::newMaterial(const char* material_type)
    {
        return (ANARIMaterial)RegisterResource(createMaterial(material_type));
    }

    ANARISampler Device::newSampler(const char* type)
    {
        return nullptr;
    }

    //--- Instancing --------------------------------------

    ANARIGroup Device::newGroup()
    {
        return (ANARIGroup)RegisterResource(std::make_unique<Group>());
    }

    ANARIInstance Device::newInstance()
    {
        return (ANARIInstance)RegisterResource(std::make_unique<Instance>());
    }

    //--- Top-level Worlds --------------------------------

    ANARIWorld Device::newWorld()
    {
        return (ANARIWorld)RegisterResource(std::make_unique<World>());
    }

    //--- Object + Parameter Lifetime Management ----------

    void Device::setParameter(ANARIObject object,
                              const char* name,
                              ANARIDataType type,
                              const void* mem)
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr)
            LOG(logging::Level::Error) << "ANARIDevice error: setting parameter on object: " << name;
        else
            obj->setParameter(name,type,mem);
    }

    void Device::unsetParameter(ANARIObject object,
                                const char* name)
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr)
            LOG(logging::Level::Error) << "ANARIDevice error: unsetting parameter on object: " << name;
        else
            obj->unsetParameter(name);
    }

    void Device::commitParameters(ANARIObject object)
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr)
            LOG(logging::Level::Error) << "ANARIDevice error: commit called on uninitialized object";
        else
            obj->commit();
    }

    void Device::release(ANARIObject object) 
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr)
            LOG(logging::Level::Error) << "ANARIDevice error: release called on uninitialized object";
        else
            obj->release();
    }

    void Device::retain(ANARIObject object)
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr)
            LOG(logging::Level::Error) << "ANARIDevice error: retain called on uninitialized object";
        else
            obj->retain();
    }

    //--- Object Query Interface --------------------------

    int Device::getProperty(ANARIObject object,
                            const char* name,
                            ANARIDataType type,
                            void* mem,
                            uint64_t size,
                            ANARIWaitMask mask)
    {
        Object* obj = (Object*)GetResource(object);
        if (obj == nullptr) {
            LOG(logging::Level::Error) << "ANARIDevice error: querying prpertion on object: " << name;
            return 0;
        } else {
            return obj->getProperty(name,type,mem,size,mask);
        }
    }

    //--- FrameBuffer Manipulation ------------------------

    ANARIFrame Device::newFrame()
    {
        return (ANARIFrame)RegisterResource(std::make_unique<Frame>());
    }

    const void* Device::frameBufferMap(ANARIFrame fb,
                                       const char* channel,
                                       uint32_t* width,
                                       uint32_t* height,
                                       ANARIDataType* pixelType)
    {
        Frame* frame = (Frame*)GetResource(fb);
        *width = frame->size[0];
        *height = frame->size[1];
        if (strncmp(channel,"channel.color",5)==0) {
            *pixelType = frame->color;
        } else if (strncmp(channel,"channel.depth",5)==0) {
            *pixelType = frame->depth;
        }
        return frame->map(channel);
    }

    void Device::frameBufferUnmap(ANARIFrame fb,
                                  const char* channel)
    {
    }

    //--- Frame Rendering ---------------------------------

    ANARIRenderer Device::newRenderer(const char* type)
    {
        return (ANARIRenderer)RegisterResource(createRenderer(type));
    }

    void Device::renderFrame(ANARIFrame frame)
    {
        Frame* f = (Frame*)GetResource(frame);
        Renderer* rend = (Renderer*)GetResource(f->renderer);

        if (rend != nullptr)
            rend->renderFrame(f);
    }

    int Device::frameReady(ANARIFrame frame,
                           ANARIWaitMask m)
    {
        Frame* f = (Frame*)GetResource(frame);
        return f->wait(m);
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
} // generic


static char deviceName[] = "generic";

extern "C" ANARI_DEFINE_LIBRARY_NEW_DEVICE(
    generic, library, subtype)
{
  if (subtype == std::string("default") || subtype == std::string("generic"))
    return (ANARIDevice) new generic::Device;
  return nullptr;
}

extern "C" ANARI_DEFINE_LIBRARY_INIT(generic)
{
  printf("...loaded generic library!\n");
}

extern "C" ANARI_DEFINE_LIBRARY_GET_DEVICE_SUBTYPES(generic, libdata)
{
  static const char *devices[] = {deviceName, nullptr};
  return devices;
}

extern "C" ANARI_DEFINE_LIBRARY_GET_OBJECT_SUBTYPES(
    generic, libdata, deviceSubtype, objectType)
{
  if (objectType == ANARI_RENDERER) {
    static const char* renderers[] = {
        "pathtracer", // 1st one is chosen by "default" 
        "ao",
        nullptr,     // last one most be NULL
    };
    return renderers;
  } else if (objectType == ANARI_VOLUME) {
    static const char* volumes[] = {
        "scivis",
        nullptr,
    };
    return volumes;
  }
  return nullptr;
}

extern "C" ANARI_DEFINE_LIBRARY_GET_OBJECT_PROPERTY(generic,
    library,
    deviceSubtype,
    objectSubtype,
    objectType,
    propertyName,
    propertyType)
{
  if (propertyType == ANARI_RENDERER) {
    return generic::Renderer::Parameters;
  }
  return nullptr;
}

extern "C" ANARI_DEFINE_LIBRARY_GET_PARAMETER_PROPERTY(generic,
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
    return (ANARIDevice) new generic::Device;
}
