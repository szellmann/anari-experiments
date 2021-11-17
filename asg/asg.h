#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <anari/anari.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASGAPI /*TODO*/

typedef int ASGError_t;
#define ASG_ERROR_NO_ERROR                  0
#define ASG_ERROR_INVALID_LUT_ID            700

typedef int ASGType_t;
#define ASG_TYPE_OBJECT                     0
#define ASG_TYPE_SURFACE                    1000
#define ASG_TYPE_LOOKUP_TABLE1D             1010
#define ASG_TYPE_STRUCTURED_VOLUME          1020
#define ASG_TYPE_INSTANCE                   1030

typedef int ASGDataType_t;
#define ASG_DATA_TYPE_UINT8                 0
#define ASG_DATA_TYPE_UINT16                1
#define ASG_DATA_TYPE_UINT32                2
#define ASG_DATA_TYPE_FLOAT32               3

typedef int ASGVisitorTraversalType_t;
#define ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN 0
#define ASG_VISITOR_TRAVERSAL_TYPE_PARENTS  1

typedef int ASGLutID;
#define ASG_LUT_ID_DEFAULT_LUT              0

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

typedef struct _ASGObject *ASGObject, *ASGSurface, *ASGLookupTable1D,
    *ASGStructuredVolume, *ASGInstance;


/* ========================================================
 * API functions
 * ========================================================*/

// Objects
ASGAPI ASGObject asgNewObject();
ASGAPI ASGError_t asgRelease(ASGObject obj);
ASGAPI ASGError_t asgGetType(ASGObject obj, ASGType_t* type);
ASGAPI ASGError_t asgObjectAddChild(ASGObject obj, ASGObject child);

// Visitor
ASGAPI ASGVisitor asgCreateVisitor(void (*visitFunc)(ASGObject, void*), void* userData);
ASGAPI ASGError_t asgDestroyVisitor(ASGVisitor visitor);
ASGAPI ASGError_t asgApplyVisitor(ASGObject obj, ASGVisitor visitor,
                                  ASGVisitorTraversalType_t traversalType);

// Surface
ASGAPI ASGSurface asgNewSurface(/*TODO*/);

// RGBA luts
ASGAPI ASGLookupTable1D asgNewLookupTable1D(float* rgb, float* alpha, int32_t numEntries,
                                            ASGFreeFunc freeFunc);
ASGAPI ASGError_t asgLookupTable1DGetRGB(ASGLookupTable1D lut, float* rgb);
ASGAPI ASGError_t asgLookupTable1DGetAlpha(ASGLookupTable1D lut, float* alpha);
ASGAPI ASGError_t asgLookupTable1DGetNumEntries(ASGLookupTable1D lut,
                                                int32_t* numEntries);

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
ASGAPI ASGError_t asgStructuredVolumeSetRange(ASGStructuredVolume vol, float rangeMin,
                                              float rangeMax);
ASGAPI ASGError_t asgStructuredVolumeGetRange(ASGStructuredVolume vol, float* rangeMin,
                                              float* rangeMax);
ASGAPI ASGError_t asgStructuredVolumeSetDist(ASGStructuredVolume, float distX,
                                             float distY, float distZ);
ASGAPI ASGError_t asgStructuredVolumeGetDist(ASGStructuredVolume, float* distX,
                                             float* distY, float* distZ);
ASGAPI ASGError_t asgStructuredVolumeSetLookupTable1D(ASGStructuredVolume vol,
                                                      ASGLookupTable1D lut);
ASGAPI ASGError_t asgStructuredVolumeGetLookupTable1D(ASGStructuredVolume vol,
                                                      ASGLookupTable1D* lut);

// I/O

// Procedural volumes, builtin RGBA LUTs, etc.
ASGAPI ASGError_t asgMakeMarschnerLobb(ASGStructuredVolume vol);
ASGAPI ASGError_t asgMakeDefaultLUT1D(ASGLookupTable1D lut, ASGLutID lutID);

// Build ANARI world
ASGAPI ASGError_t asgBuildANARIWorld(ASGObject obj, ANARIDevice device, ANARIWorld world,
                                     uint64_t flags);

#ifdef __cplusplus
}
#endif


