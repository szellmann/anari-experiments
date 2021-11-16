#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASGAPI /*TODO*/

typedef int ASGError_t;
#define ASG_ERROR_NO_ERROR                  0

typedef int ASGType_t;
#define ASG_TYPE_OBJECT                     0
#define ASG_TYPE_SURFACE                    1000
#define ASG_TYPE_STRUCTURED_VOLUME          1010
#define ASG_TYPE_INSTANCE                   1020

typedef int ASGDataType_t;
#define ASG_DATA_TYPE_UINT8                 0
#define ASG_DATA_TYPE_UINT16                1
#define ASG_DATA_TYPE_UINT32                2
#define ASG_DATA_TYPE_FLOAT32               3

typedef int ASGVisitorTraversalType_t;
#define ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN 0
#define ASG_VISITOR_TRAVERSAL_TYPE_PARENTS  1

struct _ASGObject;

typedef void (*ASGVisitFunc)(struct _ASGObject*, void*);
typedef void (*ASGFreeFunc)(void*);

typedef struct {
    ASGVisitFunc visit;
    void* userData;
} _ASGVisitor;
typedef _ASGVisitor *ASGVisitor;

/* ========================================================
 * ASGObject
 * ========================================================*/

typedef void _ASGImpl;
typedef _ASGImpl *ASGImpl;

struct _ASGObject {
    // Node interface
    ASGType_t type;
    unsigned numChildren;
    struct _ASGObject** children;
    unsigned numParents;
    struct _ASGObject** parents;

    // Visitable interface
    void (*accept)(struct _ASGObject*, ASGVisitor, ASGVisitorTraversalType_t);

    // Private implementation
    ASGImpl impl;
};
typedef struct _ASGObject *ASGObject, *ASGSurface, *ASGStructuredVolume, *ASGInstance;


/* ========================================================
 * API functions
 * ========================================================*/

// Objects
ASGAPI ASGObject asgNewObject();
ASGAPI ASGError_t asgDestroyObject(ASGObject obj);
ASGAPI ASGError_t asgObjectAddChild(ASGObject obj, ASGObject child);

ASGAPI ASGSurface asgNewSurface(/*TODO*/);

// Volumes
ASGAPI ASGStructuredVolume asgNewStructuredVolume(void* data, int32_t width,
                                                  int32_t height, int32_t depth,
                                                  ASGDataType_t type,
                                                  ASGFreeFunc freeFunc);
ASGAPI ASGError_t asgStructuredVolumeGetData(ASGStructuredVolume vol, void* data);
ASGAPI ASGError_t asgStructuredVolumeGetDims(ASGStructuredVolume vol, int32_t* width,
                                             int32_t* height, int32_t* depth);
ASGAPI ASGError_t asgStructuredVolumeGetDatatype(ASGStructuredVolume vol,
                                                 ASGDataType_t* type);

// Visitor
ASGAPI ASGVisitor asgNewVisitor(void (*visitFunc)(ASGObject, void*), void* userData);
ASGAPI ASGError_t asgApplyVisitor(ASGObject obj, ASGVisitor visitor,
                                  ASGVisitorTraversalType_t traversalType);

// I/O
ASGAPI ASGError_t asgMakeMarschnerLobb(ASGStructuredVolume vol);

#ifdef __cplusplus
}
#endif


