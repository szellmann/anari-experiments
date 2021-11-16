#include <assert.h>
#include "asg.h"

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

struct StructuredVolumeImpl {
    void* data;
    int32_t width, height, depth;
    ASGDataType_t type;
    float rangeMin, rangeMax;
    float distX, distY, distZ;
    void* userPtr;
    ASGFreeFunc freeFunc;
} _ASGStructuredVolumeImpl;

ASGStructuredVolume asgNewStructuredVolume(void* data, int32_t width, int32_t height,
                                          int32_t depth, ASGDataType_t type,
                                          ASGFreeFunc freeFunc)
{
    ASGStructuredVolume vol = (ASGStructuredVolume)asgNewObject();
    vol->type = ASG_TYPE_STRUCTURED_VOLUME;
    vol->impl = (StructuredVolumeImpl*)calloc(1,sizeof(StructuredVolumeImpl));
    ((StructuredVolumeImpl*)vol->impl)->data = data;
    ((StructuredVolumeImpl*)vol->impl)->width = width;
    ((StructuredVolumeImpl*)vol->impl)->height = height;
    ((StructuredVolumeImpl*)vol->impl)->depth = depth;
    ((StructuredVolumeImpl*)vol->impl)->type = type;
    ((StructuredVolumeImpl*)vol->impl)->rangeMin = 0.f;
    ((StructuredVolumeImpl*)vol->impl)->rangeMax = 1.f;
    ((StructuredVolumeImpl*)vol->impl)->distX = 1.f;
    ((StructuredVolumeImpl*)vol->impl)->distY = 1.f;
    ((StructuredVolumeImpl*)vol->impl)->distY = 1.f;
    ((StructuredVolumeImpl*)vol->impl)->userPtr = data;
    ((StructuredVolumeImpl*)vol->impl)->freeFunc = freeFunc;
    return vol;
}

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
// I/O
// ========================================================

ASGError_t asgMakeMarchnerLobb(ASGStructuredVolume vol)
{
    StructuredVolumeImpl* impl = (StructuredVolumeImpl*)vol->impl;
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


