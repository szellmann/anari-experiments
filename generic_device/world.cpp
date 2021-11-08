#include "array.hpp"
#include "backend.hpp"
#include "logging.hpp"
#include "world.hpp"

namespace generic {

    World::World()
        : resourceHandle(new std::remove_pointer_t<ANARIWorld>)
    {
    }

    World::~World()
    {
    }

    ResourceHandle World::getResourceHandle()
    {
        return resourceHandle;
    }

    void World::commit()
    {
        backend::commit(*this);
    }

    void World::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"volume",6)==0 && type==ANARI_ARRAY1D) {
            ANARIArray3D* handle = (ANARIArray3D*)mem;
            Array3D* arr = (Array3D*)GetResource(*handle);
            volume = (ANARIVolume*)arr->data;
            Volume* vol = (Volume*)GetResource(volume[0]); // TODO: reference count
        } else {
            LOG(logging::Level::Warning) << "World: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

} // generic


