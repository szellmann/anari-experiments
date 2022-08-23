#include <string.h>
#include <type_traits>
#include "cylindergeom.hpp"
#include "spheregeom.hpp"
#include "geometry.hpp"
#include "logging.hpp"
#include "trianglegeom.hpp"

namespace generic {

    Geometry::Geometry()
        : resourceHandle(new std::remove_pointer_t<ANARIGeometry>)
    {
    }

    Geometry::~Geometry()
    {
    }

    ResourceHandle Geometry::getResourceHandle()
    {
        return resourceHandle;
    }

    void Geometry::commit()
    {
        LOG(logging::Level::Warning) << "Calling commit on invalid geometry\n";
    }

    void Geometry::release()
    {
        LOG(logging::Level::Warning) << "Calling release on invalid geometry\n";
    }

    void Geometry::retain()
    {
        LOG(logging::Level::Warning) << "Calling retain on invalid geometry\n";
    }

    void Geometry::setParameter(const char* name,
                                ANARIDataType type,
                                const void* mem)
    {
        LOG(logging::Level::Warning) << "Setting parameter \"" << name
            << "\" on invalid geometry!";
    }

    void Geometry::unsetParameter(const char* name)
    {
        LOG(logging::Level::Warning) << "Unsetting parameter \"" << name
            << "\" on invalid geometry!";
    }

    std::unique_ptr<Geometry> createGeometry(const char* subtype)
    {
        if (strncmp(subtype,"triangle",8)==0)
            return std::make_unique<TriangleGeom>();
        else if (strncmp(subtype,"cylinder",8)==0)
            return std::make_unique<CylinderGeom>();
        else if (strncmp(subtype,"sphere",5)==0)
            return std::make_unique<SphereGeom>();
        else {
            LOG(logging::Level::Error) << "Geometry subtype unavailable: " << subtype;
            return std::make_unique<Geometry>();
        }
    }

} // generic


