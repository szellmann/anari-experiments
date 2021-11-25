#pragma once

#include <algorithm>
#include <vector>
#include <visionaray/math/math.h>
#include <vkt/InputStream.hpp>
#include <vkt/LookupTable.hpp>
#include <vkt/Resample.hpp>
#include <vkt/StructuredVolume.hpp>
#include <vkt/VolumeFile.hpp>
#include "volkit/src/vkt/TransfuncEditor.hpp"
#include <asg/asg.h>
#include <anari/anari.h>
#include "util.h"

struct Scene
{
    Scene(ANARIDevice dev, ANARIWorld wrld)
        : device(dev)
        , world(wrld)
    {
    }

    virtual ~Scene()
    {
        ASG_SAFE_CALL(asgRelease(root));
    }

    virtual visionaray::aabb getBounds()
    {
        return {};
    }

    virtual void beforeRenderFrame()
    {
    }

    virtual void afterRenderFrame()
    {
    }

    virtual void renderUI()
    {
    }

    virtual void afterRenderUI()
    {
    }

    virtual bool needFrameReset()
    {
        return false;
    }

    ANARIDevice device = nullptr;
    ANARIWorld world = nullptr;

    ASGObject root = nullptr;
};

struct SelectTest : Scene
{
    SelectTest(ANARIDevice dev, ANARIWorld wrld)
        : Scene(dev,wrld)
    {
        root = ASG_SAFE_CALL(asgNewSelect());

        static float vertex[] = {-1.f,-1.f,-1.f,
                                  1.f,-1.f,-1.f,
                                  1.f, 1.f,-1.f,
                                 -1.f, 1.f,-1.f,
                                  1.f,-1.f, 1.f,
                                 -1.f,-1.f, 1.f,
                                 -1.f, 1.f, 1.f,
                                  1.f, 1.f, 1.f};

        static uint32_t index[] = {0,1,2, 0,2,3, 4,5,6, 4,6,7,
                                   1,4,7, 1,7,2, 5,0,3, 5,3,6,
                                   5,4,1, 5,1,0, 3,2,7, 3,7,6};

        ASGTriangleGeometry boxGeom = asgNewTriangleGeometry(vertex,NULL,NULL,8,
                                                             index,12,NULL);

        // 1st
        ASGMaterial mat1 = asgNewMaterial("");
        float red[3] = {1.f,0.f,0.f};
        ASG_SAFE_CALL(asgMakeMatte(&mat1,red,NULL));
        ASG_SAFE_CALL(asgObjectSetName(mat1,"red"));
        ASGSurface surf1 = asgNewSurface(boxGeom,mat1);
        float matrix1[] = {1.f,0.f,0.f,
                           0.f,1.f,0.f,
                           0.f,0.f,1.f,
                           0.f,0.f,0.f};
        ASGTransform trans1 = asgNewTransform(matrix1);
        ASG_SAFE_CALL(asgObjectAddChild(trans1,surf1));
        ASG_SAFE_CALL(asgObjectAddChild(root,trans1));

        // 2nd
        ASGMaterial mat2 = asgNewMaterial("");
        float yellow[3] = {1.f,1.f,0.f};
        ASG_SAFE_CALL(asgMakeMatte(&mat2,yellow,NULL));
        ASG_SAFE_CALL(asgObjectSetName(mat2,"yellow"));
        ASG_SAFE_CALL(ASGSurface surf2 = asgNewSurface(boxGeom,mat2));
        float matrix2[] = {1.f,0.f,0.f,
                           0.f,1.f,0.f,
                           0.f,0.f,1.f,
                           3.f,0.f,0.f};
        ASGTransform trans2 = asgNewTransform(matrix2);
        ASG_SAFE_CALL(asgObjectAddChild(trans2,surf2));
        ASG_SAFE_CALL(asgObjectAddChild(root,trans2));

        // 3rd
        ASGMaterial mat3 = asgNewMaterial("");
        ASG_SAFE_CALL(asgObjectSetName(mat3,"green"));
        float green[3] = {0.f,1.f,0.f};
        ASG_SAFE_CALL(asgMakeMatte(&mat3,green,NULL));
        ASGSurface surf3 = asgNewSurface(boxGeom,mat3);
        float matrix3[] = {1.f,0.f,0.f,
                           0.f,1.f,0.f,
                           0.f,0.f,1.f,
                           6.f,0.f,0.f};
        ASGTransform trans3 = asgNewTransform(matrix3);
        ASG_SAFE_CALL(asgObjectAddChild(trans3,surf3));
        ASG_SAFE_CALL(asgObjectAddChild(root,trans3));

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);
    }

    visionaray::aabb getBounds()
    {
        visionaray::aabb bbox;
        bbox.invalidate();
        ASG_SAFE_CALL(asgComputeBounds(root,&bbox.min.x,&bbox.min.y,&bbox.min.z,
                                       &bbox.max.x,&bbox.max.y,&bbox.max.z,0));
        return bbox;
    }

    virtual void beforeRenderFrame()
    {
        if (rebuildANARIWorld) {
            ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                             ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

            anariCommit(device,world);
        }

        rebuildANARIWorld = false;
    }

    void renderUI()
    {
        ASGBool_t redBoxVisible, yellowBoxVisible, greenBoxVisible;
        ASG_SAFE_CALL(asgSelectGetChildVisible(root,0,&redBoxVisible));
        ASG_SAFE_CALL(asgSelectGetChildVisible(root,1,&yellowBoxVisible));
        ASG_SAFE_CALL(asgSelectGetChildVisible(root,2,&greenBoxVisible));

        bool redBoxVisibleB(redBoxVisible);
        bool yellowBoxVisibleB(yellowBoxVisible);
        bool greenBoxVisibleB(greenBoxVisible);

        ImGui::Begin("Select Test");
        if (ImGui::Checkbox("Red box", &redBoxVisibleB)) {
            ASG_SAFE_CALL(asgSelectSetChildVisible(root,0,(ASGBool_t)redBoxVisibleB));
            rebuildANARIWorld = true;
        }

        if (ImGui::Checkbox("Yellow box", &yellowBoxVisibleB)) {
            ASG_SAFE_CALL(asgSelectSetChildVisible(root,1,(ASGBool_t)yellowBoxVisibleB));
            rebuildANARIWorld = true;
        }

        if (ImGui::Checkbox("Green box", &greenBoxVisibleB)) {
            ASG_SAFE_CALL(asgSelectSetChildVisible(root,2,(ASGBool_t)greenBoxVisibleB));
            rebuildANARIWorld = true;
        }

        ImGui::End();
    }

    bool needFrameReset()
    {
        return rebuildANARIWorld;
    }

    bool rebuildANARIWorld = false;
};

// Load volume file or generate default volume
struct VolumeScene : Scene
{
    VolumeScene(ANARIDevice dev, ANARIWorld wrld, const char* fileName = NULL)
        : Scene(dev,wrld)
    {
        root = asgNewObject();

        volDims[0] = volDims[1] = volDims[2] = 0;

        if (fileName != nullptr) {
            volume = asgNewStructuredVolume(nullptr,0,0,0,ASG_DATA_TYPE_FLOAT32,nullptr);
            // load volume, resample to FLOAT32 if format is different
            ASG_SAFE_CALL(asgLoadVOLKIT(volume,fileName,ASG_IO_FLAG_RESAMPLE_VOXEL_TYPE));
            ASG_SAFE_CALL(asgStructuredVolumeGetDims(volume,&volDims[0],&volDims[1],
                                                     &volDims[2]));
        }

        if (volDims[0] == 0) { // volume wasn't loaded; generate one
            volDims[0] = volDims[1] = volDims[2] = 63;

            volData.resize(volDims[0]*volDims[1]*volDims[2]);
            volume = asgNewStructuredVolume(volData.data(),volDims[0],volDims[1],volDims[2],
                                            ASG_DATA_TYPE_FLOAT32,NULL);
            ASG_SAFE_CALL(asgMakeMarschnerLobb(volume));
        }

        rgbLUT.resize(15);
        alphaLUT.resize(5);
        lut = asgNewLookupTable1D(rgbLUT.data(),alphaLUT.data(),alphaLUT.size(),NULL);
        ASG_SAFE_CALL(asgMakeDefaultLUT1D(lut,ASG_LUT_ID_DEFAULT_LUT));
        ASG_SAFE_CALL(asgStructuredVolumeSetLookupTable1D(volume,lut));

        ASG_SAFE_CALL(asgObjectAddChild(root,volume));

        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);

        // Set up the volkit TFE
        float* rgb;
        float* alpha;
        int32_t numEntries;

        ASG_SAFE_CALL(asgLookupTable1DGetRGB(lut, &rgb));
        ASG_SAFE_CALL(asgLookupTable1DGetAlpha(lut, &alpha));
        ASG_SAFE_CALL(asgLookupTable1DGetNumEntries(lut, &numEntries));

        std::vector<float> rgba(numEntries*4);
        for (int32_t i=0; i<numEntries; ++i) {
            rgba[i*4] = rgb[i*3];
            rgba[i*4+1] = rgb[i*3+1];
            rgba[i*4+2] = rgb[i*3+2];
            rgba[i*4+3] = alpha[i];
        }

        vktLUT = vkt::LookupTable(5,1,1,vkt::ColorFormat::RGBA32F);
        vktLUT.setData((uint8_t*)rgba.data());
        tfe.setLookupTableResource(vktLUT.getResourceHandle());
    }

   ~VolumeScene()
    {
        ASG_SAFE_CALL(asgRelease(volume));
    }

    visionaray::aabb getBounds()
    {
        return {{0,0,0},{(float)volDims[0],(float)volDims[1],(float)volDims[2]}};
    }

    void afterRenderFrame()
    {
        if (tfe.updated()) {
            vkt::LookupTable* lut = tfe.getUpdatedLookupTable();
            if (lut != nullptr) {
                auto dims = lut->getDims();
                auto lutData = (float*)lut->getData();
                float* colorVals = new float[dims.x*3];
                float* alphaVals = new float[dims.x];
                for (int i=0; i<dims.x; ++i) {
                    colorVals[i*3] = lutData[i*4];
                    colorVals[i*3+1] = lutData[i*4+1];
                    colorVals[i*3+2] = lutData[i*4+2];
                    alphaVals[i] = lutData[i*4+3];
                }
                // Apply transfer function
                updateLUT(colorVals,alphaVals,dims.x);
                delete[] alphaVals;
                delete[] colorVals;

                resetFrame = true;
            }
        }
    }

    void renderUI()
    {
        ImGui::Begin("Volume");
        tfe.drawImmediate();
        ImGui::End();
    }

    bool needFrameReset()
    {
        bool temp = resetFrame;
        resetFrame = false;
        return temp;
    }

    void updateLUT(float* rgb, float* alpha, int numEntries)
    {
        rgbLUT.resize(numEntries*3);
        alphaLUT.resize(numEntries);
        std::copy(rgb,rgb+numEntries*3,rgbLUT.data());
        std::copy(alpha,alpha+numEntries,alphaLUT.data());

        ASG_SAFE_CALL(asgRelease(lut));
        lut = asgNewLookupTable1D(rgbLUT.data(),alphaLUT.data(),alphaLUT.size(),NULL);
        ASG_SAFE_CALL(asgStructuredVolumeSetLookupTable1D(volume,lut));

        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,ASG_BUILD_WORLD_FLAG_LUTS,0));
    }

    bool resetFrame = false;
    ASGStructuredVolume volume = nullptr;
    ASGLookupTable1D lut = nullptr;
    int volDims[3];
    std::vector<float> volData;
    std::vector<float> rgbLUT;
    std::vector<float> alphaLUT;
    vkt::LookupTable vktLUT;
    vkt::TransfuncEditor tfe;
};

// Load scene w/ pbrtParser (TODO)
struct PBRT : Scene {};

// Load scene w/ assimp
struct Model : Scene
{
    Model(ANARIDevice device, ANARIWorld wrld, const char* fileName)
        : Scene(device,wrld)
    {
        bbox.invalidate();

        root = asgNewObject();

        // Load from file
        ASG_SAFE_CALL(asgLoadASSIMP(root,fileName,0));

        // Compute bounding box
        ASG_SAFE_CALL(asgComputeBounds(root,&bbox.min.x,&bbox.min.y,&bbox.min.z,
                                       &bbox.max.x,&bbox.max.y,&bbox.max.z,0));

        // Assemble material list
        ASGVisitor visitor = asgCreateVisitor([](ASGVisitor self, ASGObject obj,
                                                 void* userData) {
            std::vector<ASGMaterial>& materials
                        = *((std::vector<ASGMaterial>*)userData);
            ASGType_t t;
            ASG_SAFE_CALL(asgGetType(obj,&t));

            if (t==ASG_TYPE_SURFACE) {
                ASGMaterial mat;
                ASG_SAFE_CALL(asgSurfaceGetMaterial(obj,&mat));
                if (mat != nullptr && std::find(materials.begin(),materials.end(),mat)
                                == materials.end())
                    materials.push_back(mat);
            }

            ASG_SAFE_CALL(asgVisitorApply(self,obj));
        },&materials,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN);
        ASG_SAFE_CALL(asgObjectAccept(root,visitor));

        float matrix[] = {1.f,0.f,0.f,
                          0.f,1.f,0.f,
                          0.f,0.f,1.f,
                          0.f,0.f,0.f};
        ASGTransform trans = asgNewTransform(matrix);
        ASG_SAFE_CALL(asgObjectAddChild(trans,root));

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(/*root*/trans,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    virtual void beforeRenderFrame()
    {
        if (rebuildANARIWorld) {
            ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                             ASG_BUILD_WORLD_FLAG_MATERIALS,0));

            anariCommit(device,world);
        }

        rebuildANARIWorld = false;
    }

    virtual void renderUI()
    {
        ImGui::Begin("Materials");

        static const char* current_item = NULL;
        static ASGMaterial mat = NULL;

        if (ImGui::BeginCombo("##combo", current_item))
        {
            for (size_t m=0; m<materials.size(); ++m)
            {
                const char* name;
                ASG_SAFE_CALL(asgObjectGetName(materials[m],&name));
                bool is_selected = (current_item == name);
                if (ImGui::Selectable(name, is_selected)) {
                    mat = materials[m];
                    current_item = name;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        float kd[3] {0.f,0.f,0.f};
        if (mat != NULL) {
            ASGParam kdParam;
            ASG_SAFE_CALL(asgMaterialGetParam(mat,"kd",&kdParam));
            ASG_SAFE_CALL(asgParamGetValue(kdParam,kd));
        }

        if (ImGui::ColorEdit3("Diffuse Color",kd,
                    ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_Float))
        {
            if (mat != NULL) {
                ASG_SAFE_CALL(asgMaterialSetParam(mat,asgParam3fv("kd",kd)));
                rebuildANARIWorld = true;
            }
        }
        ImGui::End();
    }

    bool needFrameReset()
    {
        return rebuildANARIWorld;
    }

    visionaray::aabb bbox;

    std::vector<ASGMaterial> materials;

    bool rebuildANARIWorld = false;
};


