#include <memory>
#include <stdexcept>

namespace asg {

    // ====================================================
    // Param
    // ====================================================

    Param::Param(const char* name, int v1, int v2, int v3, int v4)
    { parm = asgParam4i(name,v1,v2,v3,v4); }

    Param::Param(const char* name, float v1, float v2, float v3)
    { parm = asgParam3f(name,v1,v2,v3); }

    Param::operator ASGParam() const { return parm; }

    namespace detail {

        inline void errorFunc(Error error)
        {
            // TODO!
            if (error != ASG_ERROR_NO_ERROR)
                throw std::runtime_error("Error code: "+std::to_string((int)error));
        }



        // ==========================================================
        // Object
        // ==========================================================

        Object::Object(ASGObject obj)
            : handle(obj)
        {
        }

        std::shared_ptr<Object> Object::create()
        {
            return std::shared_ptr<Object>(new Object(asgNewObject()));
        }

        Object::operator::ASGObject()
        {
            return handle;
        }

        void Object::release()
        {
            detail::errorFunc(asgRelease(handle));
        }

        void Object::retain()
        {
            detail::errorFunc(asgRetain(handle));
        }

        template <typename T>
        inline void Object::addChild(const std::shared_ptr<T>& child)
        {
            detail::errorFunc(asgObjectAddChild(handle,child.get()->handle));
        }

        inline std::shared_ptr<Object> Object::getChild(int childID)
        {
            ASGObject child;
            detail::errorFunc(asgObjectGetChild(handle,childID,&child));
            return std::shared_ptr<Object>(new Object(child));
        }



        // ==========================================================
        // Geometry
        // ==========================================================

        Geometry::Geometry(ASGGeometry obj)
            : Object(obj)
        {
        }

        Geometry::operator ASGGeometry()
        {
            return (ASGGeometry)handle;
        }


        // ==========================================================
        // TriangleGeometry
        // ==========================================================

        TriangleGeometry::TriangleGeometry(ASGGeometry obj)
            : Geometry(obj)
        {
        }

        std::shared_ptr<TriangleGeometry> TriangleGeometry::create(float* vertices,
                                                                   float* vertexNormals,
                                                                   float* vertexColors,
                                                                   uint32_t numVertices,
                                                                   uint32_t* indices,
                                                                   uint32_t numIndices,
                                                                   FreeFunc freeVertices,
                                                                   FreeFunc freeNormals,
                                                                   FreeFunc freeColors,
                                                                   FreeFunc freeIndices)
        {
            return std::shared_ptr<TriangleGeometry>(new TriangleGeometry(
                asgNewTriangleGeometry(vertices,vertexNormals,vertexColors,numVertices,
                                       indices,numIndices,freeVertices,freeNormals,
                                       freeColors,freeIndices)));
        }

        TriangleGeometry::operator ASGTriangleGeometry()
        {
            return (ASGTriangleGeometry)handle;
        }


        // ==========================================================
        // CylinderGeometry
        // ==========================================================

        CylinderGeometry::CylinderGeometry(ASGGeometry obj)
            : Geometry(obj)
        {
        }

        std::shared_ptr<CylinderGeometry> CylinderGeometry::create(float* vertices,
                                                                   float* radii,
                                                                   float* vertexColors,
                                                                   uint8_t* caps,
                                                                   uint32_t numVertices,
                                                                   uint32_t* indices,
                                                                   uint32_t numIndices,
                                                                   float defaultRadius,
                                                                   FreeFunc freeVertices,
                                                                   FreeFunc freeRadii,
                                                                   FreeFunc freeColors,
                                                                   FreeFunc freeCaps,
                                                                   FreeFunc freeIndices)
        {
            return std::shared_ptr<CylinderGeometry>(new CylinderGeometry(
                asgNewCylinderGeometry(vertices,radii,vertexColors,caps,numVertices,
                                       indices,numIndices,defaultRadius,freeVertices,
                                       freeRadii,freeColors,freeCaps,freeIndices)));
        }


        // ==========================================================
        // Material
        // ==========================================================

        Material::Material(ASGMaterial obj)
            : Object(obj)
        {
        }

        std::shared_ptr<Material> Material::create(const char* materialType)
        {
            return std::shared_ptr<Material>(new Material(asgNewMaterial(materialType)));
        }

        Material::operator ASGMaterial()
        {
            return (ASGMaterial)handle;
        }

        void Material::setParam(Param parm)
        {
            detail::errorFunc(asgMaterialSetParam(handle,(ASGParam)parm));
        }

        // ==========================================================
        // Surface
        // ==========================================================

        Surface::Surface(ASGSurface obj)
            : Object(obj)
        {
        }

        std::shared_ptr<Surface> Surface::create(ASGGeometry geom, ASGMaterial mat)
        {
            return std::shared_ptr<Surface>(new Surface(asgNewSurface(geom,mat)));
        }


        // ==========================================================
        // Transform
        // ==========================================================

        Transform::Transform(ASGTransform obj)
            : Object(obj)
        {
        }

        std::shared_ptr<Transform> Transform::create(float initialMatrix[12],
                                                     MatrixFormat format)
        {
            if (initialMatrix == nullptr) {
                float I[] = {1.f,0.f,0.f,
                             0.f,1.f,0.f,
                             0.f,0.f,1.f,
                             0.f,0.f,0.f};
                return std::shared_ptr<Transform>(new Transform(
                    asgNewTransform(I,format)));
            } else {
                return std::shared_ptr<Transform>(new Transform(
                    asgNewTransform(initialMatrix,format)));
            }
        }

        void Transform::translate(float x, float y, float z)
        {
            float xyz[] = {x,y,z};
            translate(xyz);
        }

        void Transform::translate(float xyz[3])
        {
            detail::errorFunc(asgTransformTranslate(handle,xyz));
        }
    } // detail
} // ::asg


