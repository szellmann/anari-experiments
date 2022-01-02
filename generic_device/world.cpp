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

    void World::release()
    {
    }

    void World::retain()
    {
    }

    void World::setParameter(const char* name,
                             ANARIDataType type,
                             const void* mem)
    {
        if (strncmp(name,"instance",8)==0 && type==ANARI_ARRAY1D) {
            instance = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"surface",7)==0 && type==ANARI_ARRAY1D) {
            surface = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"volume",6)==0 && type==ANARI_ARRAY1D) {
            volume = *(ANARIArray1D*)mem; // TODO: reference count
        } else if (strncmp(name,"light",5)==0 && type==ANARI_ARRAY1D) {
            light = *(ANARIArray1D*)mem; // TODO: reference count
        } else {
            LOG(logging::Level::Warning) << "World: Unsupported parameter "
                << "/ parameter type: " << name << " / " << type;
        }
    }

    void World::unsetParameter(const char* name)
    {
        if (strncmp(name,"instance",8)==0) {
            instance = nullptr;
        } else if (strncmp(name,"surface",7)==0) {
            surface = nullptr;
        } else if (strncmp(name,"volume",6)==0) {
            volume = nullptr;
        } else if (strncmp(name,"light",5)==0) {
            light = nullptr;
        } else {
            LOG(logging::Level::Warning) << "World: Unsupported parameter " << name;
        }
    }

} // generic


