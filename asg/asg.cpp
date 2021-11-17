#include <assert.h>
#include <vector>
#include "asg.h"

// ========================================================
// Ref-counted objects
// ========================================================

struct _ASGObject {
    // Node interface
    ASGType_t type;
    unsigned numChildren;
    struct _ASGObject** children;
    unsigned numParents;
    struct _ASGObject** parents;

    // Ref count (TODO!)

    // Visitable interface
    void (*accept)(struct _ASGObject*, ASGVisitor, ASGVisitorTraversalType_t);

    // Private implementation
    ASGImpl impl;

    // Updated/used by visitors
    bool dirty;
    uint64_t mask;
};

void _asgAccept(struct _ASGObject* _obj, ASGVisitor _visitor,
                ASGVisitorTraversalType_t traversalType)
{
    _visitor->visit(_obj, _visitor->userData);

    ASGObject obj = (ASGObject)_obj;

    if (traversalType==ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN) {
        for (unsigned i=0; i<obj->numChildren; ++i) {
            ASGObject child = ((ASGObject*)(obj->children))[i];
            child->accept(child,_visitor,traversalType);
        }
    } else {
        for (unsigned i=0; i<obj->numParents; ++i) {
            ASGObject parent = ((ASGObject*)(obj->parents))[i];
            parent->accept(parent,_visitor,traversalType);
        }
    }
}

ASGObject asgNewObject()
{
    ASGObject obj = (ASGObject)calloc(1,sizeof(struct _ASGObject));
    obj->type = ASG_TYPE_OBJECT;
    obj->numChildren = 0;
    obj->children = NULL;
    obj->numParents = 0;
    obj->parents = NULL;
    obj->accept = _asgAccept;
    obj->impl = NULL;
    obj->dirty = false;
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
    obj->numChildren++;
    obj->children = (struct _ASGObject**)realloc(obj->children,obj->numChildren);
    // TODO: check if reallocation failed
    ((ASGObject*)obj->children)[obj->numChildren-1] = child;

    child->numParents++;
    child->parents = (struct _ASGObject**)realloc(child->parents,child->numParents);
    ((ASGObject*)child->parents)[child->numParents-1] = obj;

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
    lut->dirty = true;
    ((LUT1D*)lut->impl)->rgb = rgb;
    ((LUT1D*)lut->impl)->alpha = alpha;
    ((LUT1D*)lut->impl)->numEntries = numEntries;
    ((LUT1D*)lut->impl)->rgbUserPtr = rgb;
    ((LUT1D*)lut->impl)->alphaUserPtr = alpha;
    ((LUT1D*)lut->impl)->freeFunc = freeFunc;
    return lut;
}

ASGError_t asgLookupTable1DGetRGB(ASGLookupTable1D lut, float* rgb)
{
    rgb = ((LUT1D*)lut->impl)->rgb;
    return ASG_ERROR_NO_ERROR;
}

ASGError_t asgLookupTable1DGetAlpha(ASGLookupTable1D lut, float* alpha)
{
    alpha = ((LUT1D*)lut->impl)->alpha;
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
};

ASGStructuredVolume asgNewStructuredVolume(void* data, int32_t width, int32_t height,
                                          int32_t depth, ASGDataType_t type,
                                          ASGFreeFunc freeFunc)
{
    ASGStructuredVolume vol = (ASGStructuredVolume)asgNewObject();
    vol->type = ASG_TYPE_STRUCTURED_VOLUME;
    vol->impl = (StructuredVolume*)calloc(1,sizeof(StructuredVolume));
    vol->dirty = true;
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

ASGError_t asgStructuredVolumeGetData(ASGStructuredVolume vol, void* data)
{
    data = ((StructuredVolume*)vol->impl)->data;
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

// ========================================================
// Builtin visitors
// ========================================================

struct ANARIArraySizes {
    int numVolumes;
};

struct ANARI {
    ANARIDevice device;
    ANARIWorld world;
    std::vector<ANARIVolume> volumes;
};

static void visitANARIWorld(ASGObject obj, void* userData) {

    ANARI* anari = (ANARI*)userData;
    ASGType_t t;
    asgGetType(obj,&t);

    switch (t)
    {
        case ASG_TYPE_OBJECT: break;
        case ASG_TYPE_SURFACE: break;
        case ASG_TYPE_STRUCTURED_VOLUME: {
            StructuredVolume* impl = (StructuredVolume*)obj->impl;
            if (obj->dirty) {
                obj->dirty = false;
                anariRelease(anari->device,impl->anariVolume);
                impl->anariVolume = anariNewVolume(anari->device,"scivis");

                assert(impl->type == ASG_DATA_TYPE_FLOAT32); // TODO!

                int volDims[] = {
                    impl->width,
                    impl->height,
                    impl->depth,
                };

                ANARIArray3D scalar = anariNewArray3D(anari->device,impl->data,
                                                      0,0,ANARI_FLOAT32,
                                                      volDims[0],volDims[1],volDims[2],
                                                      0,0,0);

                ANARISpatialField field = anariNewSpatialField(anari->device,
                                                               "structuredRegular");
                anariSetParameter(anari->device, field,"data",ANARI_ARRAY3D,&scalar);
                const char* filter = "linear";
                anariSetParameter(anari->device,field,"filter",ANARI_STRING,filter);
                anariCommit(anari->device, field);

                float valueRange[2] = {0.f,1.f};
                //anariSetParameter(anari->device,impl->anariVolume,"valueRange",ANARI_FLOAT32_BOX1,&valueRange);

                anariSetParameter(anari->device,impl->anariVolume,"field",
                                  ANARI_SPATIAL_FIELD, &field);

                anariRelease(anari->device, scalar);
                anariRelease(anari->device, field);

                anari->volumes.push_back(impl->anariVolume);
            }

            if (impl->lut1D != NULL && impl->lut1D->dirty) {
                impl->lut1D->dirty = false;
            } else if (impl->lut1D == NULL) {
                int32_t numEntries = 5;
                float* rgb = (float*)malloc(numEntries*3*sizeof(float));
                float* alpha = (float*)malloc(numEntries*sizeof(float));
                impl->lut1D = asgNewLookupTable1D(rgb,alpha,numEntries,free);
                asgMakeDefaultLUT1D(impl->lut1D,ASG_LUT_ID_DEFAULT_LUT);
                impl->lut1D->dirty = false;
            }

            break;
        }
        default: break;
    }
}

ASGError_t asgBuildANARIWorld(ASGObject obj, ANARIDevice device, ANARIWorld world,
                              uint64_t flags)
{
    // flags ignored for now, but could be used to, e.g., only extract volumes, etc.

    ANARI anari;
    anari.device = device;
    anari.world = world;
    ASGVisitor visitor = asgCreateVisitor(visitANARIWorld,&anari);
    asgApplyVisitor(obj,visitor,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
    asgDestroyVisitor(visitor);

    if (anari.volumes.size() > 0) {
        ANARIArray1D volumes = anariNewArray1D(device,anari.volumes.data(),0,0,
                                               ANARI_VOLUME,anari.volumes.size(),0);
        anariSetParameter(device,world,"volume",ANARI_ARRAY1D,&volumes);
        anariRelease(device,volumes);
    }

    return ASG_ERROR_NO_ERROR;
}


