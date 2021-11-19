#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#if ASG_HAVE_ASSIMP
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/Matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#endif
#if ASG_HAVE_VOLKIT
#include <vkt/InputStream.hpp>
#include <vkt/Resample.hpp>
#include <vkt/StructuredVolume.hpp>
#include <vkt/VolumeFile.hpp>
#endif
#include "asg.h"

// ========================================================
// Data types
// ========================================================

struct TypeInfo {
    ASGDataType_t type;
    size_t size;
};

static const TypeInfo g_typeInfo[] = {
    { ASG_DATA_TYPE_INT8,                        1  },
    { ASG_DATA_TYPE_INT8_VEC1,                   1  },
    { ASG_DATA_TYPE_INT8_VEC2,                   2  },
    { ASG_DATA_TYPE_INT8_VEC3,                   3  },
    { ASG_DATA_TYPE_INT8_VEC4,                   4  },

    { ASG_DATA_TYPE_INT16,                       2  },
    { ASG_DATA_TYPE_INT16_VEC1,                  2  },
    { ASG_DATA_TYPE_INT16_VEC2,                  4  },
    { ASG_DATA_TYPE_INT16_VEC3,                  6  },
    { ASG_DATA_TYPE_INT16_VEC4,                  8  },

    { ASG_DATA_TYPE_INT32,                       4  },
    { ASG_DATA_TYPE_INT32_VEC1,                  4  },
    { ASG_DATA_TYPE_INT32_VEC2,                  8  },
    { ASG_DATA_TYPE_INT32_VEC3,                 12  },
    { ASG_DATA_TYPE_INT32_VEC4,                 16  },

    { ASG_DATA_TYPE_INT64,                       8  },
    { ASG_DATA_TYPE_INT64_VEC1,                  8  },
    { ASG_DATA_TYPE_INT64_VEC2,                 16  },
    { ASG_DATA_TYPE_INT64_VEC3,                 24  },
    { ASG_DATA_TYPE_INT64_VEC4,                 32  },

    { ASG_DATA_TYPE_UINT8,                       1  },
    { ASG_DATA_TYPE_UINT8_VEC1,                  1  },
    { ASG_DATA_TYPE_UINT8_VEC2,                  2  },
    { ASG_DATA_TYPE_UINT8_VEC3,                  3  },
    { ASG_DATA_TYPE_UINT8_VEC4,                  4  },

    { ASG_DATA_TYPE_UINT16,                      2  },
    { ASG_DATA_TYPE_UINT16_VEC1,                 2  },
    { ASG_DATA_TYPE_UINT16_VEC2,                 4  },
    { ASG_DATA_TYPE_UINT16_VEC3,                 6  },
    { ASG_DATA_TYPE_UINT16_VEC4,                 8  },

    { ASG_DATA_TYPE_UINT32,                      4  },
    { ASG_DATA_TYPE_UINT32_VEC1,                 4  },
    { ASG_DATA_TYPE_UINT32_VEC2,                 8  },
    { ASG_DATA_TYPE_UINT32_VEC3,                12  },
    { ASG_DATA_TYPE_UINT32_VEC4,                16  },

    { ASG_DATA_TYPE_UINT64,                      8  },
    { ASG_DATA_TYPE_UINT64_VEC1,                 8  },
    { ASG_DATA_TYPE_UINT64_VEC2,                16  },
    { ASG_DATA_TYPE_UINT64_VEC3,                24  },
    { ASG_DATA_TYPE_UINT64_VEC4,                32  },

    { ASG_DATA_TYPE_FLOAT32,                     4  },
    { ASG_DATA_TYPE_FLOAT32_VEC1,                4  },
    { ASG_DATA_TYPE_FLOAT32_VEC2,                8  },
    { ASG_DATA_TYPE_FLOAT32_VEC3,               12  },
    { ASG_DATA_TYPE_FLOAT32_VEC4,               16  },

    { ASG_DATA_TYPE_HANDLE,         sizeof(void*)   },
};

size_t asgSizeOfDataType(ASGDataType_t type)
{
    auto begin = g_typeInfo;
    auto end = g_typeInfo + sizeof(g_typeInfo)/sizeof(TypeInfo);
    auto it = std::find_if(begin,end,[type](TypeInfo info)
                                     { return info.type==type; });

    if (it != end)
        return it->size;

    return size_t(-1);
}

// ========================================================
// ASGParam
// ========================================================

#define PARAM_DEF1(TYPE,TYPENAME,MNEMONIC)                                              \
ASGParam asgParam1##MNEMONIC(const char* name, TYPE v1) {                               \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME;                                                              \
    std::memcpy(&param.value,&v1,asgSizeOfDataType(TYPENAME));                          \
    return param;                                                                       \
}

#define PARAM_DEF2(TYPE,TYPENAME,MNEMONIC)                                              \
ASGParam asgParam1##MNEMONIC(const char* name, TYPE v1, TYPE v2) {                      \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME##_VEC2;                                                       \
    size_t sodt = asgSizeOfDataType(TYPENAME);                                          \
    std::memcpy(&param.value,&v1,sodt);                                                 \
    std::memcpy(&param.value+sodt,&v2,sodt);                                            \
    return param;                                                                       \
}

#define PARAM_DEF3(TYPE,TYPENAME,MNEMONIC)                                              \
ASGParam asgParam1##MNEMONIC(const char* name, TYPE v1, TYPE v2, TYPE v3) {             \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME##_VEC3;                                                       \
    size_t sodt = asgSizeOfDataType(TYPENAME);                                          \
    std::memcpy(&param.value,&v1,sodt);                                                 \
    std::memcpy(&param.value+sodt,&v2,sodt);                                            \
    std::memcpy(&param.value+2*sodt,&v3,sodt);                                          \
    return param;                                                                       \
}

#define PARAM_DEF4(TYPE,TYPENAME,MNEMONIC)                                              \
ASGParam asgParam1##MNEMONIC(const char* name, TYPE v1, TYPE v2, TYPE v3, TYPE v4) {    \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME##_VEC4;                                                       \
    size_t sodt = asgSizeOfDataType(TYPENAME);                                          \
    std::memcpy(&param.value,&v1,sodt);                                                 \
    std::memcpy(&param.value+sodt,&v2,sodt);                                            \
    std::memcpy(&param.value+2*sodt,&v3,sodt);                                          \
    std::memcpy(&param.value+3*sodt,&v4,sodt);                                          \
    return param;                                                                       \
}

#define PARAM_DEF2V(TYPE,TYPENAME,MNEMONIC)                                             \
ASGParam asgParam2##MNEMONIC(const char* name, TYPE* v) {                               \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME;                                                              \
    std::memcpy(&param.value,v,asgSizeOfDataType(TYPENAME));                            \
    return param;                                                                       \
}

#define PARAM_DEF3V(TYPE,TYPENAME,MNEMONIC)                                             \
ASGParam asgParam3##MNEMONIC(const char* name, TYPE* v) {                               \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME;                                                              \
    std::memcpy(&param.value,v,asgSizeOfDataType(TYPENAME));                            \
    return param;                                                                       \
}

#define PARAM_DEF4V(TYPE,TYPENAME,MNEMONIC)                                             \
ASGParam asgParam4##MNEMONIC(const char* name, TYPE* v) {                               \
    ASGParam param;                                                                     \
    param.name = name;                                                                  \
    param.type = TYPENAME;                                                              \
    std::memcpy(&param.value,v,asgSizeOfDataType(TYPENAME));                            \
    return param;                                                                       \
}

PARAM_DEF1(int,ASG_DATA_TYPE_INT32,i)
PARAM_DEF2(int,ASG_DATA_TYPE_INT32,i)
PARAM_DEF3(int,ASG_DATA_TYPE_INT32,i)
PARAM_DEF4(int,ASG_DATA_TYPE_INT32,i)
PARAM_DEF2V(int,ASG_DATA_TYPE_INT32_VEC2,iv)
PARAM_DEF3V(int,ASG_DATA_TYPE_INT32_VEC3,iv)
PARAM_DEF4V(int,ASG_DATA_TYPE_INT32_VEC3,iv)

PARAM_DEF1(float,ASG_DATA_TYPE_FLOAT32,f)
PARAM_DEF2(float,ASG_DATA_TYPE_FLOAT32,f)
PARAM_DEF3(float,ASG_DATA_TYPE_FLOAT32,f)
PARAM_DEF4(float,ASG_DATA_TYPE_FLOAT32,f)
PARAM_DEF2V(float,ASG_DATA_TYPE_FLOAT32_VEC2,fv)
PARAM_DEF3V(float,ASG_DATA_TYPE_FLOAT32_VEC3,fv)
PARAM_DEF4V(float,ASG_DATA_TYPE_FLOAT32_VEC3,fv)

ASGParam asgParamSampler2D(const char* name, ASGSampler2D samp)
{
    ASGParam param;
    param.name = name;
    param.type = ASG_DATA_TYPE_HANDLE;
    // TODO: this stuff here needs validation...
    if (samp == nullptr)
        std::memset(&param.value,0,sizeof(param.value));
    else
        std::memcpy(&param.value,samp,asgSizeOfDataType(ASG_DATA_TYPE_HANDLE));
    return param;
}

ASGError_t asgParamGetValue(ASGParam param, void* mem)
{
    std::memcpy(mem,&param.value,asgSizeOfDataType(param.type));
    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// Ref-counted objects
// ========================================================

struct _ASGObject {
    // Node interface
    ASGType_t type;
    std::vector<_ASGObject*> children;
    std::vector<_ASGObject*> parents;

    // Ref count (TODO!)

    // Visitable interface
    void (*accept)(struct _ASGObject*, ASGVisitor, ASGVisitorTraversalType_t);

    // Private implementation
    ASGImpl impl;

    // Used by visitors to, e.g., cull nodes
    uint64_t mask;
};

void _asgAccept(struct _ASGObject* _obj, ASGVisitor _visitor,
                ASGVisitorTraversalType_t traversalType)
{
    _visitor->visit(_obj, _visitor->userData);

    ASGObject obj = (ASGObject)_obj;

    if (traversalType==ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN) {
        for (unsigned i=0; i<obj->children.size(); ++i) {
            ASGObject child = obj->children[i];
            child->accept(child,_visitor,traversalType);
        }
    } else {
        for (unsigned i=0; i<obj->parents.size(); ++i) {
            ASGObject parent = obj->parents[i];
            parent->accept(parent,_visitor,traversalType);
        }
    }
}

ASGObject asgNewObject()
{
    ASGObject obj = (ASGObject)calloc(1,sizeof(struct _ASGObject));
    obj->type = ASG_TYPE_OBJECT;
    obj->accept = _asgAccept;
    obj->impl = NULL;
    obj->mask = 0ULL;
    return obj;
}

ASGError_t asgGetType(ASGObject obj, ASGType_t* type)
{
    *type = obj->type;

    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgRelease(ASGObject obj)
{
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgRetain(ASGObject obj)
{
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgObjectAddChild(ASGObject obj, ASGObject child)
{
    obj->children.push_back(child);
    child->parents.push_back(obj);

    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// Visitor
// ========================================================

ASGVisitor asgCreateVisitor(void (*visitFunc)(ASGObject, void*), void* userData)
{
    ASGVisitor visitor = (ASGVisitor)calloc(1,sizeof(_ASGVisitor));
    visitor->visit = (ASGVisitFunc)visitFunc;
    visitor->userData = userData;
    return visitor;
}

ASGError_t asgDestroyVisitor(ASGVisitor visitor)
{
    free(visitor);
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgApplyVisitor(ASGObject self, ASGVisitor visitor,
                           ASGVisitorTraversalType_t traversalType)
{
    self->accept(self,visitor,traversalType);
    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// Material
// ========================================================

struct Material {
    std::string type;
    std::string name;
    std::vector<ASGParam> params;
    // Exclusively used by ANARI build visitors
    ANARIMaterial anariMaterial = NULL;
};

ASGMaterial asgNewMaterial(const char* materialType, const char* name)
{
    ASGMaterial material = (ASGMaterial)asgNewObject();
    material->type = ASG_TYPE_MATERIAL;
    material->impl = (Material*)calloc(1,sizeof(Material));
    ((Material*)material->impl)->type = materialType;
    ((Material*)material->impl)->name = name;
    return material;
}

ASGError_t asgMaterialGetType(ASGMaterial material, const char** materialType)
{
    *materialType = ((Material*)material->impl)->type.c_str();
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgMaterialGetName(ASGMaterial material, const char** name)
{
    *name = ((Material*)material->impl)->name.c_str();
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgMaterialSetParam(ASGMaterial material, ASGParam param)
{
    auto it = std::find_if(((Material*)material->impl)->params.begin(),
                           ((Material*)material->impl)->params.end(),
                           [param](ASGParam p) {
                                size_t n=std::max(strlen(param.name),strlen(p.name));
                                return strncmp(param.name,p.name,n)==0;
                           });

    if (it == ((Material*)material->impl)->params.end()) {
        ((Material*)material->impl)->params.push_back(param);
    } else {
        *it = param;
    }

    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgMaterialGetParam(ASGMaterial material, const char* paramName,
                               ASGParam* param)
{
    auto it = std::find_if(((Material*)material->impl)->params.begin(),
                           ((Material*)material->impl)->params.end(),
                           [paramName](ASGParam p) {
                                size_t n=std::max(strlen(paramName),strlen(p.name));
                                return strncmp(paramName,p.name,n)==0;
                           });

    if (it != ((Material*)material->impl)->params.end()) {
        ASGParam p = *it;
        std::memcpy(param,&p,sizeof(ASGParam));
        return ASG_ERROR_NO_ERROR;
    }

    return ASG_ERROR_PARAM_NOT_FOUND;
}

// ========================================================
// Geometries
// ========================================================

struct TriangleGeom {
    float* vertices;
    float* vertexNormals;
    float* vertexColors;
    uint32_t numVertices;
    uint32_t* indices;
    uint32_t numIndices;
    float* verticesUserPtr;
    float* vertexNormalsUserPtr;
    float* vertexColorsUserPtr;
    uint32_t* indicesUserPtr;
    ASGFreeFunc freeFunc;
    // Exclusively used by ANARI build visitors
    ANARIGeometry anariGeometry = NULL;
};

ASGTriangleGeometry asgNewTriangleGeometry(float* vertices, float* vertexNormals,
                                           float* vertexColors, uint32_t numVertices,
                                           uint32_t* indices, uint32_t numIndices,
                                           ASGFreeFunc freeFunc)
{
    ASGTriangleGeometry geom = (ASGTriangleGeometry)asgNewObject();
    geom->type = ASG_TYPE_TRIANGLE_GEOMETRY;
    geom->impl = (TriangleGeom*)calloc(1,sizeof(TriangleGeom));
    ((TriangleGeom*)geom->impl)->vertices = vertices;
    ((TriangleGeom*)geom->impl)->vertexNormals = vertexNormals;
    ((TriangleGeom*)geom->impl)->vertexColors = vertexColors;
    ((TriangleGeom*)geom->impl)->numVertices = numVertices;
    ((TriangleGeom*)geom->impl)->indices = indices;
    ((TriangleGeom*)geom->impl)->numIndices = numIndices;
    ((TriangleGeom*)geom->impl)->verticesUserPtr = vertices;
    ((TriangleGeom*)geom->impl)->vertexNormalsUserPtr = vertexNormals;
    ((TriangleGeom*)geom->impl)->vertexColorsUserPtr = vertexColors;
    ((TriangleGeom*)geom->impl)->indicesUserPtr = indices;
    ((TriangleGeom*)geom->impl)->freeFunc = freeFunc;
    return geom;
}

// ========================================================
// Surface
// ========================================================

struct Surface {
    ASGGeometry geometry;
    ASGMaterial material;
    // Exclusively used by ANARI build visitors
    ANARISurface anariSurface = NULL;
};

ASGSurface asgNewSurface(ASGGeometry geom, ASGMaterial mat)
{
    ASGSurface surf = (ASGSurface)asgNewObject();
    surf->type = ASG_TYPE_SURFACE;
    surf->impl = (Surface*)calloc(1,sizeof(Surface));
    ((Surface*)surf->impl)->geometry = geom;
    ((Surface*)surf->impl)->material = mat;
    return surf;
}

ASGGeometry asgSurfaceGetGeometry(ASGSurface surf, ASGGeometry* geom)
{
    *geom = ((Surface*)surf->impl)->geometry;
    return ASG_ERROR_NO_ERROR;
}

ASGMaterial asgSurfaceGetMaterial(ASGSurface surf, ASGMaterial* mat)
{
    *mat = ((Surface*)surf->impl)->material;
    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// RGBA luts
// ========================================================

struct LUT1D {
    float* rgb;
    float* alpha;
    int32_t numEntries;
    float* rgbUserPtr;
    float* alphaUserPtr;
    ASGFreeFunc freeFunc;
};

ASGLookupTable1D asgNewLookupTable1D(float* rgb, float* alpha, int32_t numEntries,
                                     ASGFreeFunc freeFunc)
{
    ASGLookupTable1D lut = (ASGLookupTable1D)asgNewObject();
    lut->type = ASG_TYPE_LOOKUP_TABLE1D;
    lut->impl = (LUT1D*)calloc(1,sizeof(LUT1D));
    ((LUT1D*)lut->impl)->rgb = rgb;
    ((LUT1D*)lut->impl)->alpha = alpha;
    ((LUT1D*)lut->impl)->numEntries = numEntries;
    ((LUT1D*)lut->impl)->rgbUserPtr = rgb;
    ((LUT1D*)lut->impl)->alphaUserPtr = alpha;
    ((LUT1D*)lut->impl)->freeFunc = freeFunc;
    return lut;
}

ASGError_t asgLookupTable1DGetRGB(ASGLookupTable1D lut, float** rgb)
{
    *rgb = ((LUT1D*)lut->impl)->rgb;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgLookupTable1DGetAlpha(ASGLookupTable1D lut, float** alpha)
{
    *alpha = ((LUT1D*)lut->impl)->alpha;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgLookupTable1DGetNumEntries(ASGLookupTable1D lut, int32_t* numEntries)
{
    *numEntries = ((LUT1D*)lut->impl)->numEntries;
    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// Volumes
// ========================================================

struct StructuredVolume {
    void* data;
    int32_t width, height, depth;
    ASGDataType_t type;
    float rangeMin, rangeMax;
    float distX, distY, distZ;
    ASGLookupTable1D lut1D;
    void* userPtr;
    ASGFreeFunc freeFunc;
    // Exclusively used by ANARI build visitors
    ANARIVolume anariVolume = NULL;
    ANARISpatialField anariSpatialField = NULL;
};

ASGStructuredVolume asgNewStructuredVolume(void* data, int32_t width, int32_t height,
                                          int32_t depth, ASGDataType_t type,
                                          ASGFreeFunc freeFunc)
{
    ASGStructuredVolume vol = (ASGStructuredVolume)asgNewObject();
    vol->type = ASG_TYPE_STRUCTURED_VOLUME;
    vol->impl = (StructuredVolume*)calloc(1,sizeof(StructuredVolume));
    ((StructuredVolume*)vol->impl)->data = data;
    ((StructuredVolume*)vol->impl)->width = width;
    ((StructuredVolume*)vol->impl)->height = height;
    ((StructuredVolume*)vol->impl)->depth = depth;
    ((StructuredVolume*)vol->impl)->type = type;
    ((StructuredVolume*)vol->impl)->rangeMin = 0.f;
    ((StructuredVolume*)vol->impl)->rangeMax = 1.f;
    ((StructuredVolume*)vol->impl)->distX = 1.f;
    ((StructuredVolume*)vol->impl)->distY = 1.f;
    ((StructuredVolume*)vol->impl)->distY = 1.f;
    ((StructuredVolume*)vol->impl)->lut1D = NULL;
    ((StructuredVolume*)vol->impl)->userPtr = data;
    ((StructuredVolume*)vol->impl)->freeFunc = freeFunc;
    return vol;
}

ASGError_t asgStructuredVolumeGetData(ASGStructuredVolume vol, void** data)
{
    *data = ((StructuredVolume*)vol->impl)->data;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeGetDims(ASGStructuredVolume vol, int32_t* width,
                                      int32_t* height, int32_t* depth)
{
    *width = ((StructuredVolume*)vol->impl)->width;
    *height = ((StructuredVolume*)vol->impl)->height;
    *depth = ((StructuredVolume*)vol->impl)->depth;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeGetType(ASGStructuredVolume vol, ASGType_t* type)
{
    *type = ((StructuredVolume*)vol->impl)->type;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeSetRange(ASGStructuredVolume vol, float rangeMin,
                                       float rangeMax)
{
    ((StructuredVolume*)vol->impl)->rangeMin = rangeMin;
    ((StructuredVolume*)vol->impl)->rangeMax = rangeMax;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeGetRange(ASGStructuredVolume vol, float* rangeMin,
                                       float* rangeMax)
{
    *rangeMin = ((StructuredVolume*)vol->impl)->rangeMin;
    *rangeMax = ((StructuredVolume*)vol->impl)->rangeMax;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeSetDist(ASGStructuredVolume vol, float distX, float distY,
                                      float distZ)
{
    ((StructuredVolume*)vol->impl)->distX = distX;
    ((StructuredVolume*)vol->impl)->distY = distY;
    ((StructuredVolume*)vol->impl)->distZ = distZ;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeGetDist(ASGStructuredVolume vol, float* distX, float* distY,
                                      float* distZ)
{
    *distX = ((StructuredVolume*)vol->impl)->distX;
    *distY = ((StructuredVolume*)vol->impl)->distY;
    *distZ = ((StructuredVolume*)vol->impl)->distZ;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeSetLookupTable1D(ASGStructuredVolume vol,
                                               ASGLookupTable1D lut)
{
    StructuredVolume* impl = (StructuredVolume*)vol->impl;
    if (impl->lut1D != NULL)
        asgRelease(impl->lut1D);

    impl->lut1D = lut;
    asgRetain(lut);
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgStructuredVolumeGetLookupTable1D(ASGStructuredVolume vol,
                                               ASGLookupTable1D* lut)
{
    *lut = ((StructuredVolume*)vol->impl)->lut1D;
    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// I/O
// ========================================================

namespace assimp {

    typedef uint64_t VisitFlags;

    VisitFlags FLAG_GEOMETRY         = 1;
    VisitFlags FLAG_ACCUM_TRANSFORMS = 2;

    void Visit(aiNode* node, const aiScene* scene, ASGObject obj,
               const std::vector<ASGMaterial>& materials, aiMatrix4x4 accTransform,
               VisitFlags flags)
    {
        aiMatrix4x4 transform;

        if (node->mNumMeshes > 0) {
            if (flags & FLAG_GEOMETRY) {
                for (unsigned i=0; i<node->mNumMeshes; ++i) {
                    unsigned meshID = node->mMeshes[i];
                    aiMesh* mesh = scene->mMeshes[meshID];

                    float* vertices = (float*)malloc(mesh->mNumVertices*3*sizeof(float));

                    float* vertexNormals = NULL;
                    if (mesh->HasNormals())
                        vertexNormals
                                = (float*)malloc(mesh->mNumVertices*4*sizeof(float));

                    float* vertexColors = NULL;
                    if (mesh->mColors[0] != NULL) // TODO:(?) AI_MAX_NUMBER_OF_COLOR_SETS
                        vertexColors
                                = (float*)malloc(mesh->mNumVertices*3*sizeof(float));

                    for (unsigned j=0; j<mesh->mNumVertices; ++j) {
                        aiVector3D v = mesh->mVertices[j];

                        if (flags & FLAG_ACCUM_TRANSFORMS) {
                            v *= accTransform;
                        }

                        vertices[j*3] = v.x;
                        vertices[j*3+1] = v.y;
                        vertices[j*3+2] = v.z;

                        if (vertexNormals != NULL) {
                            aiVector3D vn = mesh->mNormals[j];

                            // TODO: accum. inverse-transpose, too
                            if (flags & FLAG_ACCUM_TRANSFORMS) {
                                //vn *= accTransform;
                            }

                            vertexNormals[j*3] = vn.x;
                            vertexNormals[j*3+1] = vn.y;
                            vertexNormals[j*3+2] = vn.z;
                        }

                        if (vertexColors != NULL) {
                            aiColor4D vc = mesh->mColors[0][j];
                            vertexColors[j*4] = vc.r;
                            vertexColors[j*4+1] = vc.g;
                            vertexColors[j*4+2] = vc.b;
                            vertexColors[j*4+3] = vc.a;
                        }
                    }

                    uint32_t* indices = NULL;
                    uint32_t numIndices = 0;
                    if (mesh->HasFaces()) {
                        numIndices = mesh->mNumFaces;
                        // The following is valid as we're importing with triangulation
                        indices = (uint32_t*)malloc(numIndices*3*sizeof(uint32_t));
                        for (unsigned j=0; j<mesh->mNumFaces; ++j) {
                            indices[j*3] = mesh->mFaces[j].mIndices[0];
                            indices[j*3+1] = mesh->mFaces[j].mIndices[1];
                            indices[j*3+2] = mesh->mFaces[j].mIndices[2];
                        }
                    }

                    ASGMaterial mat = nullptr;

                    unsigned materialID = mesh->mMaterialIndex;

                    if (materialID < materials.size()) { // TODO: error handling
                        mat = materials[materialID];
                    }

                    ASGTriangleGeometry geom = asgNewTriangleGeometry(vertices,
                                                                      vertexNormals,
                                                                      vertexColors,
                                                                      mesh->mNumVertices,
                                                                      indices,
                                                                      numIndices,
                                                                      free);

                    ASGSurface surf = asgNewSurface(geom,mat);

                    asgObjectAddChild(obj,surf);
                }
                transform = aiMatrix4x4(); // identity
            }
        } else {
            if (flags & FLAG_ACCUM_TRANSFORMS) {
                transform = node->mTransformation * accTransform;
            }
        }

        for (unsigned i=0; i<node->mNumChildren; ++i) {
            if (flags & FLAG_ACCUM_TRANSFORMS) // flatten
                Visit(node->mChildren[i],scene,obj,materials,transform,flags);
        }
    }
} // ::assimp

ASGError_t asgLoadASSIMP(ASGObject obj, const char* fileName, uint64_t flags)
{
#if ASG_HAVE_ASSIMP
    Assimp::DefaultLogger::create("",Assimp::Logger::VERBOSE);

    static Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(fileName,
                                             aiProcess_Triangulate |
                                             //aiProcess_JoinIdenticalVertices  |
                                             aiProcess_SortByPType);

    if (scene == nullptr) {
        Assimp::DefaultLogger::get()->error(importer.GetErrorString());
        return ASG_ERROR_FILE_IO_ERROR;
    }

    std::vector<ASGMaterial> materials;
    for (unsigned i=0; i<scene->mNumMaterials; ++i) {
        ASGMaterial mat = asgNewMaterial("","");

        aiMaterial* assimpMAT = scene->mMaterials[i];
        aiColor3D col;
        assimpMAT->Get(AI_MATKEY_COLOR_DIFFUSE,col);
        aiString name;
        assimpMAT->Get(AI_MATKEY_NAME,name);

        float kd[3] = {col.r,col.g,col.b};

        asgMakeMatte(&mat,name.C_Str(),kd,NULL);
        materials.push_back(mat);
        Material* m = (Material*)mat->impl;
    }
    assimp::VisitFlags vflags = assimp::FLAG_GEOMETRY | assimp::FLAG_ACCUM_TRANSFORMS;
    assimp::Visit(scene->mRootNode,scene,obj,materials,aiMatrix4x4(),vflags);

    return ASG_ERROR_NO_ERROR;
#else
    return ASG_ERROR_MISSING_FILE_HANDLER;
#endif
}

ASGError_t asgLoadVOLKIT(ASGStructuredVolume vol, const char* fileName, uint64_t flags)
{
#if ASG_HAVE_VOLKIT
    vkt::VolumeFile file(fileName, vkt::OpenMode::Read);

    vkt::VolumeFileHeader hdr = file.getHeader();

    if (!hdr.isStructured)
        return ASG_ERROR_FILE_IO_ERROR;

    vkt::Vec3i dims = hdr.dims;
    if (dims.x * dims.y * dims.z < 1)
        return ASG_ERROR_FILE_IO_ERROR;

    vkt::DataFormat dataFormat = hdr.dataFormat;
    if (dataFormat == vkt::DataFormat::Unspecified)
    {
        // just guessing here..
        dataFormat = vkt::DataFormat::UInt8;
    }

    vkt::StructuredVolume inputVolume(dims.x, dims.y, dims.z, dataFormat);
    vkt::InputStream is(file);
    is.read(inputVolume);

    StructuredVolume* impl = (StructuredVolume*)vol->impl;

    vkt::StructuredVolume volkitVolume = inputVolume;
    if (flags & ASG_IO_FLAG_RESAMPLE_VOXEL_TYPE) {
        vkt::DataFormat destDataFormat = vkt::DataFormat::UInt8;
        if (impl->type == ASG_DATA_TYPE_UINT8)
            destDataFormat = vkt::DataFormat::UInt16;
        else if (impl->type == ASG_DATA_TYPE_UINT16)
            destDataFormat = vkt::DataFormat::UInt16;
        else if (impl->type == ASG_DATA_TYPE_FLOAT32)
            destDataFormat = vkt::DataFormat::Float32;
        else
            return ASG_ERROR_FILE_IO_ERROR;

        if (destDataFormat != dataFormat) {
            volkitVolume = vkt::StructuredVolume(dims.x, dims.y, dims.z, destDataFormat);
            vkt::Resample(volkitVolume,inputVolume,vkt::FilterMode::Nearest);
            hdr.dataFormat = dataFormat = destDataFormat;
        }
    }
    // TODO: same for volume dims

    size_t bpv(-1);
    if (dataFormat == vkt::DataFormat::UInt8)
        bpv = 1;
    else if (dataFormat == vkt::DataFormat::UInt16)
        bpv = 2;
    else if (dataFormat == vkt::DataFormat::Float32)
        bpv = 4;
    else
        return ASG_ERROR_FILE_IO_ERROR;

    asgRelease(vol);

    impl->data = malloc(dims.x * dims.y * dims.z * bpv);
    std::memcpy(impl->data,volkitVolume.getData(),dims.x * dims.y * dims.z * bpv);
    impl->width = dims.x;
    impl->height = dims.y;
    impl->depth = dims.z;
    impl->type = bpv==1 ? ASG_DATA_TYPE_UINT8
                        : bpv==2 ? ASG_DATA_TYPE_UINT16 : ASG_DATA_TYPE_FLOAT32;
    impl->rangeMin = volkitVolume.getVoxelMapping().x;
    impl->rangeMax = volkitVolume.getVoxelMapping().y;
    impl->distX = volkitVolume.getDist().x;
    impl->distY = volkitVolume.getDist().x;
    impl->distZ = volkitVolume.getDist().z;
    impl->lut1D = NULL; // TODO: file might actually have a lut
    impl->userPtr = impl->data;
    impl->freeFunc = free;
    return ASG_ERROR_NO_ERROR;
#else
    return ASG_ERROR_MISSING_FILE_HANDLER;
#endif
}

// ========================================================
// Procedural
// ========================================================

ASGError_t asgMakeMarschnerLobb(ASGStructuredVolume vol)
{
    StructuredVolume* impl = (StructuredVolume*)vol->impl;
    int32_t volDims[3] = {
        impl->width,
        impl->height,
        impl->depth,
    };

    float fM = 6.f, a = .25f;
    auto rho = [=](float r) {
        return cosf(2.f*M_PI*fM*cosf(M_PI*r/2.f));
    };
    for (int z=0; z<volDims[2]; ++z) {
        for (int y=0; y<volDims[1]; ++y) {
            for (int x=0; x<volDims[0]; ++x) {

                float xx = (float)x/volDims[0]*2.f-1.f;
                float yy = (float)y/volDims[1]*2.f-1.f;
                float zz = (float)z/volDims[2]*2.f-1.f;
                unsigned linearIndex = volDims[0]*volDims[1]*z
                    + volDims[0]*y
                    + x;

                float val = (1.f-sinf(M_PI*zz/2.f) + a * (1+rho(sqrtf(xx*xx+yy*yy))))
                                            / (2.f*(1.f+a));

                assert(impl->type==ASG_DATA_TYPE_FLOAT32);
                ((float*)impl->data)[linearIndex] = val;
            }
        }
    }

    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgMakeDefaultLUT1D(ASGLookupTable1D lut, ASGLutID lutID)
{
    switch (lutID)
    {
        case ASG_LUT_ID_DEFAULT_LUT: {
            int numEntries = ((LUT1D*)lut->impl)->numEntries;
            assert(numEntries == 5);

            float rgba[] = {
                    1.f, 1.f, 1.f, .005f,
                    0.f, .1f, .1f, .25f,
                    .5f, .5f, .7f, .5f,
                    .7f, .7f, .07f, .75f,
                    1.f, .3f, .3f, 1.f
                    };
            for (int i=0; i<numEntries; ++i) {
                ((LUT1D*)lut->impl)->rgb[i*3] = rgba[i*4];
                ((LUT1D*)lut->impl)->rgb[i*3+1] = rgba[i*4+1];
                ((LUT1D*)lut->impl)->rgb[i*3+2] = rgba[i*4+2];
                ((LUT1D*)lut->impl)->alpha[i] = rgba[i*4+3];
            }
            break;
        }

        default: return ASG_ERROR_INVALID_LUT_ID;
    }

    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgMakeMatte(ASGMaterial* material, const char* name, float kd[3],
                        ASGSampler2D mapKD)
{
    asgRelease(*material);
    *material = asgNewMaterial("matte",name);

    asgMaterialSetParam(*material,asgParam3fv("kd",kd));
    asgMaterialSetParam(*material,asgParamSampler2D("mapKD",mapKD));

    return ASG_ERROR_NO_ERROR;
}

// ========================================================
// Builtin visitors
// ========================================================

// Compute bounds

struct Bounds {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

static void visitBounds(ASGObject obj, void* userData) {

    ASGType_t t;
    asgGetType(obj,&t);

    Bounds* bounds = (Bounds*)userData;

    switch (t)
    {
        case ASG_TYPE_OBJECT: break;
        case ASG_TYPE_SURFACE: {
            Surface* surf = (Surface*)obj->impl;
            assert(surf->geometry != nullptr);

            if (surf->geometry->type == ASG_TYPE_TRIANGLE_GEOMETRY) {

                TriangleGeom* geom = (TriangleGeom*)surf->geometry->impl;

                if (geom->indices != nullptr && geom->numIndices > 0) {
                    for (unsigned i=0; i<geom->numIndices; ++i) {
                        float* v = &geom->vertices[geom->indices[i]*3];
                        bounds->minX = std::min(bounds->minX,v[0]);
                        bounds->minY = std::min(bounds->minX,v[1]);
                        bounds->minZ = std::min(bounds->minX,v[2]);
                        bounds->maxX = std::max(bounds->maxX,v[0]);
                        bounds->maxY = std::max(bounds->maxX,v[1]);
                        bounds->maxZ = std::max(bounds->maxX,v[2]);
                    }
                } else {
                    // No indices, so just insert the verts directly
                    for (unsigned i=0; i<geom->numVertices; ++i) {
                        float* v = &geom->vertices[i*3];
                        bounds->minX = std::min(bounds->minX,v[0]);
                        bounds->minY = std::min(bounds->minX,v[1]);
                        bounds->minZ = std::min(bounds->minX,v[2]);
                        bounds->maxX = std::max(bounds->maxX,v[0]);
                        bounds->maxY = std::max(bounds->maxX,v[1]);
                        bounds->maxZ = std::max(bounds->maxX,v[2]);
                    }
                }
            }

            break;
        }

        case ASG_TYPE_STRUCTURED_VOLUME: {
            // TODO!
            break;
        }
        default: break;
    }
}

ASGError_t asgComputeBounds(ASGObject obj, float* minX, float* minY, float* minZ,
                            float* maxX, float* maxY, float* maxZ, uint64_t nodeMask)
{
    Bounds bounds{+FLT_MAX,+FLT_MAX,+FLT_MAX,
                  -FLT_MAX,-FLT_MAX,-FLT_MAX};
    ASGVisitor visitor = asgCreateVisitor(visitBounds,&bounds);
    asgApplyVisitor(obj,visitor,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
    asgDestroyVisitor(visitor);

    *minX = bounds.minX; *minY = bounds.minY; *minZ = bounds.minZ;
    *maxX = bounds.maxX; *maxY = bounds.maxY; *maxZ = bounds.maxZ;

    return ASG_ERROR_NO_ERROR;
}


// Build ANARI world

struct ANARIArraySizes {
    int numVolumes;
};

struct ANARI {
    ANARIDevice device;
    ANARIWorld world;
    ASGBuildWorldFlags_t flags;
    std::vector<ANARISurface> surfaces;
    std::vector<ANARIVolume> volumes;
};

static void visitANARIWorld(ASGObject obj, void* userData) {

    ANARI* anari = (ANARI*)userData;
    ASGType_t t;
    asgGetType(obj,&t);

    switch (t)
    {
        case ASG_TYPE_OBJECT: break;
        case ASG_TYPE_SURFACE: {
            Surface* surf = (Surface*)obj->impl;

            assert(surf->geometry != nullptr);
            TriangleGeom* geom = (TriangleGeom*)surf->geometry->impl;

            if ((anari->flags & ASG_BUILD_WORLD_FLAG_GEOMETRIES) &&
                surf->geometry->type == ASG_TYPE_TRIANGLE_GEOMETRY) {

                if (geom->anariGeometry == nullptr)
                    geom->anariGeometry
                        = anariNewGeometry(anari->device,"triangle");

                ANARIArray1D vertexPosition = anariNewArray1D(anari->device,
                                                              geom->vertices,
                                                              0,0,ANARI_FLOAT32_VEC3,
                                                              geom->numVertices,0);

                anariSetParameter(anari->device,geom->anariGeometry,"vertex.position",
                                  ANARI_ARRAY1D,&vertexPosition);

                anariRelease(anari->device,vertexPosition);

                if (geom->indices != nullptr && geom->numIndices > 0) {
                    ANARIArray1D primitiveIndex = anariNewArray1D(anari->device,
                                                                  geom->indices,
                                                                  0,0,
                                                                  ANARI_UINT32_VEC3,
                                                                  geom->numIndices,
                                                                  0);
                    anariSetParameter(anari->device,geom->anariGeometry,
                                      "primitive.index",
                                      ANARI_ARRAY1D,&primitiveIndex);

                    anariRelease(anari->device,primitiveIndex);
                }

                anariCommit(anari->device,geom->anariGeometry);
            }

            if ((anari->flags & ASG_BUILD_WORLD_FLAG_MATERIALS)
                    && surf->material != nullptr) {
                Material* mat = (Material*)surf->material->impl;

                if (strncmp(mat->type.c_str(),"matte",5)==0) {
                    ASGParam kdParam;
                    ASGError_t res = asgMaterialGetParam(surf->material,"kd",
                                                         &kdParam);
                    if (res != ASG_ERROR_PARAM_NOT_FOUND) {
                        if (mat->anariMaterial == nullptr)
                            mat->anariMaterial = anariNewMaterial(anari->device, "matte");

                        float kd[3];
                        asgParamGetValue(kdParam,kd);
                        //std::cout << kd[0] << ' ' << kd[1] << ' ' << kd[2] << '\n';

                        anariSetParameter(anari->device,mat->anariMaterial,"color",
                                          ANARI_FLOAT32_VEC3,kd);
                        anariCommit(anari->device,mat->anariMaterial);
                    }
                }
            }

            if (surf->anariSurface == nullptr)
                surf->anariSurface = anariNewSurface(anari->device);

            anariSetParameter(anari->device,surf->anariSurface,"geometry",
                              ANARI_GEOMETRY,&geom->anariGeometry);
            if (surf->material != nullptr
                 && ((Material*)(surf->material->impl))->anariMaterial != nullptr) {
                Material* mat = (Material*)surf->material->impl;
                anariSetParameter(anari->device,surf->anariSurface,"material",
                                  ANARI_MATERIAL,&mat->anariMaterial);
            }
            anariCommit(anari->device,surf->anariSurface);

            anari->surfaces.push_back(surf->anariSurface);

            break;
        }

        case ASG_TYPE_STRUCTURED_VOLUME: {
            StructuredVolume* vol = (StructuredVolume*)obj->impl;

            if (vol->anariVolume == nullptr)
                vol->anariVolume = anariNewVolume(anari->device,"scivis");

            if (anari->flags & ASG_BUILD_WORLD_FLAG_VOLUMES) {

                assert(vol->type == ASG_DATA_TYPE_FLOAT32); // TODO!

                int volDims[] = {
                    vol->width,
                    vol->height,
                    vol->depth,
                };

                anariRelease(anari->device, vol->anariSpatialField);

                ANARIArray3D scalar = anariNewArray3D(anari->device,vol->data,
                                                      0,0,ANARI_FLOAT32,
                                                      volDims[0],volDims[1],volDims[2],
                                                      0,0,0);

                vol->anariSpatialField = anariNewSpatialField(anari->device,
                                                              "structuredRegular");
                anariSetParameter(anari->device,vol->anariSpatialField,"data",
                                  ANARI_ARRAY3D,&scalar);
                const char* filter = "linear";
                anariSetParameter(anari->device,vol->anariSpatialField,"filter",
                                  ANARI_STRING,filter);
                anariCommit(anari->device, vol->anariSpatialField);

                float valueRange[2] = {0.f,1.f};
                //anariSetParameter(anari->device,vol->anariVolume,"valueRange",ANARI_FLOAT32_BOX1,&valueRange);

                anariSetParameter(anari->device,vol->anariVolume,"field",
                                  ANARI_SPATIAL_FIELD, &vol->anariSpatialField);

                anariCommit(anari->device, vol->anariVolume);

                anariRelease(anari->device, scalar);
            }

            if (anari->flags & ASG_BUILD_WORLD_FLAG_LUTS) {
                float* rgb;
                float* alpha;
                int32_t numEntries;

                if (vol->lut1D == NULL) {
                    numEntries = 5;
                    rgb = (float*)malloc(numEntries*3*sizeof(float));
                    alpha = (float*)malloc(numEntries*sizeof(float));
                    vol->lut1D = asgNewLookupTable1D(rgb,alpha,numEntries,free);
                    asgMakeDefaultLUT1D(vol->lut1D,ASG_LUT_ID_DEFAULT_LUT);
                } else {
                    asgLookupTable1DGetRGB(vol->lut1D, &rgb);
                    asgLookupTable1DGetAlpha(vol->lut1D, &alpha);
                    asgLookupTable1DGetNumEntries(vol->lut1D, &numEntries);
                }

                ANARIArray1D anariColor = anariNewArray1D(anari->device, rgb, 0, 0, ANARI_FLOAT32_VEC3, numEntries, 0);
                ANARIArray1D anariOpacity = anariNewArray1D(anari->device, alpha, 0, 0, ANARI_FLOAT32, numEntries, 0);

                anariSetParameter(anari->device, vol->anariVolume, "color", ANARI_ARRAY1D, &anariColor);
                anariSetParameter(anari->device, vol->anariVolume, "opacity", ANARI_ARRAY1D, &anariOpacity);

                anariCommit(anari->device, vol->anariVolume);

                anariRelease(anari->device, anariColor);
                anariRelease(anari->device, anariOpacity);
            }

            anari->volumes.push_back(vol->anariVolume);

            break;
        }
        default: break;
    }
}

ASGError_t asgBuildANARIWorld(ASGObject obj, ANARIDevice device, ANARIWorld world,
                              ASGBuildWorldFlags_t flags, uint64_t nodeMask)
{
    if (flags == 0)
        flags = ASG_BUILD_WORLD_FLAG_FULL_REBUILD;

    ANARI anari;
    anari.device = device;
    anari.world = world;
    anari.flags = flags;
    ASGVisitor visitor = asgCreateVisitor(visitANARIWorld,&anari);
    asgApplyVisitor(obj,visitor,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
    asgDestroyVisitor(visitor);

    if (anari.surfaces.size() > 0) {
        ANARIArray1D surfaces = anariNewArray1D(device,anari.surfaces.data(),0,0,
                                                ANARI_SURFACE,anari.surfaces.size(),0);
        anariSetParameter(device,world,"surface",ANARI_ARRAY1D,&surfaces);
        anariRelease(device,surfaces);
    }

    if (anari.volumes.size() > 0) {
        ANARIArray1D volumes = anariNewArray1D(device,anari.volumes.data(),0,0,
                                               ANARI_VOLUME,anari.volumes.size(),0);
        anariSetParameter(device,world,"volume",ANARI_ARRAY1D,&volumes);
        anariRelease(device,volumes);
    }

    return ASG_ERROR_NO_ERROR;
}


