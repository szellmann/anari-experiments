#include <string.h>
#include <type_traits>
#include "logging.hpp"
#include "material.hpp"
#include "matte.hpp"

namespace generic {

    Material::Material()
    {
    }

    Material::~Material()
    {
    }

    ResourceHandle Material::getResourceHandle()
    {
        return resourceHandle;
    }

    void Material::commit()
    {
        LOG(logging::Level::Warning) << "Calling commit on invalid material\n";
    }

    void Material::release()
    {
        LOG(logging::Level::Warning) << "Calling release on invalid material\n";
    }

    void Material::retain()
    {
        LOG(logging::Level::Warning) << "Calling retain on invalid material\n";
    }

    void Material::setParameter(const char* name,
                                ANARIDataType type,
                                const void* mem)
    {
        LOG(logging::Level::Warning) << "Setting parameter \"" << name
            << "\" on invalid material!";
    }

    std::unique_ptr<Material> createMaterial(const char* subtype)
    {
        if (strncmp(subtype,"matte",5)==0)
            return std::make_unique<Matte>();
        else {
            LOG(logging::Level::Error) << "Material subtype unavailable: " << subtype;
            return std::make_unique<Material>();
        }
    }

} // generic


