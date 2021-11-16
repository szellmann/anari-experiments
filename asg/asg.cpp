#include <assert.h>
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

ASGVisitor asgNewVisitor(void (*visitFunc)(ASGObject, void*), void* userData)
{
    ASGVisitor visitor = (ASGVisitor)calloc(1,sizeof(_ASGVisitor));
    visitor->visit = (ASGVisitFunc)visitFunc;
    visitor->userData = userData;
    return visitor;
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
    ((LUT1D*)lut->impl)->rgb = rgb;
    ((LUT1D*)lut->impl)->alpha = alpha;
    ((LUT1D*)lut->impl)->numEntries = numEntries;
    ((LUT1D*)lut->impl)->rgbUserPtr = rgb;
    ((LUT1D*)lut->impl)->alphaUserPtr = alpha;
    ((LUT1D*)lut->impl)->freeFunc = freeFunc;
    return lut;
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


