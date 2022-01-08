#pragma once

#include <algorithm>
#include <memory>
#include <random>
#include <vector>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <visionaray/math/math.h>
#include <visionaray/pinhole_camera.h>
#include <common/manip/rotate_manipulator.h>
#include <common/manip/translate_manipulator.h>
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

    virtual void renderUI(visionaray::pinhole_camera const&)
    {
    }

    virtual void afterRenderUI()
    {
    }

    virtual bool handleResize(int width, int height)
    {
        viewportWidth = width;
        viewportHeight = height;
        return false;
    }

    virtual bool handleMouseDown(visionaray::mouse_event const&)
    {
        return false;
    }

    virtual bool handleMouseUp(visionaray::mouse_event const&)
    {
        return false;
    }

    virtual bool handleMouseMove(visionaray::mouse_event const&)
    {
        return false;
    }

    virtual bool needFrameReset()
    {
        return false;
    }

    ANARIDevice device = nullptr;
    ANARIWorld world = nullptr;

    ASGObject root = nullptr;

    int viewportWidth = 0;
    int viewportHeight = 0;
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
                        ASG_BUILD_WORLD_FLAG_FULL_REBUILD & ~ASG_BUILD_WORLD_FLAG_LIGHTS,0));

            anariCommit(device,world);
        }

        rebuildANARIWorld = false;
    }

    void renderUI(visionaray::pinhole_camera const&)
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

struct SphereTest : Scene
{
    SphereTest(ANARIDevice dev, ANARIWorld wrld)
        : Scene(dev,wrld)
    {
        root = asgNewObject();

        uint32_t numSpheres = 10000;

        std::vector<float> positions(numSpheres*3);
        std::vector<float> radii(numSpheres);
        std::vector<float> colors(numSpheres*4);
        std::vector<uint32_t> indices(numSpheres);

        std::default_random_engine rnd;
        std::uniform_real_distribution<float> pos(-1.0f, 1.0f);
        std::uniform_real_distribution<float> rad(.005f, .01f);

        for (uint32_t i=0; i<numSpheres; ++i) {
            positions[i*3] = pos(rnd);
            positions[i*3+1] = pos(rnd);
            positions[i*3+2] = pos(rnd);
            radii[i] = rad(rnd);
            unsigned r = (unsigned)(i*13*17 + 0x234235);
            unsigned g = (unsigned)(i*7*3*5 + 0x773477);
            unsigned b = (unsigned)(i*11*19 + 0x223766);
            colors[i*4]   = (r&255)/255.f;
            colors[i*4+1] = (g&255)/255.f;
            colors[i*4+2] = (b&255)/255.f;
            colors[i*4+3] = 1.f;
            indices[i] = i;
        }

        // w/ indices
        // ASGSphereGeometry sphereGeom = asgNewSphereGeometry(positions.data(),radii.data(),
        //                                                     NULL,numSpheres,
        //                                                     indices.data(),
        //                                                     (uint32_t)indices.size());

        // w/o indices
        ASGSphereGeometry sphereGeom = asgNewSphereGeometry(positions.data(),radii.data(),
                                                            colors.data(),numSpheres,NULL,
                                                            0);

        ASGMaterial mat = asgNewMaterial("");
        float grey[3] = {.8f,.8f,.8f};
        ASG_SAFE_CALL(asgMakeMatte(&mat,grey,NULL));
        ASG_SAFE_CALL(asgObjectSetName(mat,"grey"));

        ASGSurface spheres = asgNewSurface(sphereGeom,mat);
        ASG_SAFE_CALL(asgObjectAddChild(root,spheres));


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

    void renderUI(visionaray::pinhole_camera const&)
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

        anariCommit(device,world);
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

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    virtual bool handleMouseDown(visionaray::mouse_event const& event)
    {
        if (event.pos().y < toolbarHeight)
            return true;

        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_down(event)) {
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_down(event)) {
                return true;
            }
        }

        picked.downPos = event.pos();
        return false;
    }

    virtual bool handleMouseUp(visionaray::mouse_event const& event)
    {
        if (event.pos().y < toolbarHeight)
            return true;

        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_up(event)) {
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_up(event)) {
                return true;
            }
        }

        if (manip == 1 && picked.downPos == event.pos()) {
            pickObject(event.pos());
            doDelete = doRotate = doMove = false;
        }

        return false;
    }

    virtual bool handleMouseMove(visionaray::mouse_event const& event)
    {
        visionaray::mat4x3 m4x3;
        m4x3.col0 = modelMatrix.col0.xyz();
        m4x3.col1 = modelMatrix.col1.xyz();
        m4x3.col2 = modelMatrix.col2.xyz();
        m4x3.col3 = modelMatrix.col3.xyz();

        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_move(event)) {
                ASG_SAFE_CALL(asgTransformSetMatrix(picked.transform,m4x3.data()));
                ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                                 ASG_BUILD_WORLD_FLAG_TRANSFORMS,0));
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_move(event)) {
                ASG_SAFE_CALL(asgTransformSetMatrix(picked.transform,m4x3.data()));
                ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                                 ASG_BUILD_WORLD_FLAG_TRANSFORMS,0));
                return true;
            }
        }

        return Scene::handleMouseMove(event);
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

    void pickObject(visionaray::mouse::pos mousePos) {
        float aspect = cam.aspect();
        float pos[3] = { cam.eye().x, cam.eye().y, cam.eye().z };
        float view[3] = { cam.center().x-cam.eye().x,
                          cam.center().y-cam.eye().y,
                          cam.center().z-cam.eye().z };
        float up[3] = { cam.up().x, cam.up().y, cam.up().z };
        ASGCamera cam = asgNewCamera("perspective");
        ASG_SAFE_CALL(asgCameraSetParam(cam,asgParam1f("aspect",aspect)));
        ASG_SAFE_CALL(asgCameraSetParam(cam,asgParam3fv("position",pos)));
        ASG_SAFE_CALL(asgCameraSetParam(cam,asgParam3fv("direction",view)));
        ASG_SAFE_CALL(asgCameraSetParam(cam,asgParam3fv("up",up)));

        // Pick in high-res frame:
        uint32_t scale = 16;
        uint32_t w = viewportWidth*scale;
        uint32_t h = viewportHeight*scale;
        uint32_t x = mousePos.x*scale+scale/2;
        uint32_t y = mousePos.y*scale+scale/2;

        ASGObject pickedObject = NULL;
        ASG_SAFE_CALL(asgPickObject(root,cam,x,h-y-1,w,h,&pickedObject));
        ASG_SAFE_CALL(asgRelease(cam));std::cout << pickedObject << '\n';

        if (pickedObject != NULL) {
            ASGType_t type;
            ASG_SAFE_CALL(asgGetType(pickedObject,&type));

            if (type == ASG_TYPE_SURFACE) {
                ASGGeometry geom;
                // TODO: is triangle geom?
                ASG_SAFE_CALL(asgSurfaceGetGeometry(pickedObject,&geom));

                createTriangleGeomPipelineGL(picked.glPipeline);
                updateTriangleGeomPipelineGL(geom,picked.glPipeline);
                picked.handle = pickedObject;
                picked.geomHandle = geom;

                ASGObject parent;
                ASG_SAFE_CALL(asgObjectGetParent(pickedObject,0,&parent));
                if (parent != NULL) {
                    ASGType_t ptype;
                    ASG_SAFE_CALL(asgGetType(parent,&ptype));
                    if (ptype==ASG_TYPE_TRANSFORM) {
                        picked.transform = parent;

                        float matrix[12];
                        ASG_SAFE_CALL(asgTransformGetMatrix(parent,matrix));

                        modelMatrix = visionaray::mat4(visionaray::mat4x3(matrix));
                    } else {
                        modelMatrix = visionaray::mat4::identity();
                    }
                } else {
                    modelMatrix = visionaray::mat4::identity();
                }
            }
        } else {
            picked.transform = NULL;
            picked.handle = NULL;
            picked.geomHandle = NULL;
        }
    }

    void ToolbarUI()
    {
        int menuBarHeight = 0;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarHeight));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = 0
            | ImGuiWindowFlags_NoDocking 
            | ImGuiWindowFlags_NoTitleBar 
            | ImGuiWindowFlags_NoResize 
            | ImGuiWindowFlags_NoMove 
            | ImGuiWindowFlags_NoScrollbar 
            | ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::Begin("TOOLBAR", NULL, window_flags);
        ImGui::PopStyleVar();

        ImGui::RadioButton("None",   &manip, 0); ImGui::SameLine();
        ImGui::RadioButton("Pick",   &manip, 1); ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::RadioButton("Rotate", &manip, 2); ImGui::SameLine();
        ImGui::EndDisabled();
        ImGui::RadioButton("Move",   &manip, 3); ImGui::SameLine();
        doDelete = ImGui::Button("Delete");

        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    }

    virtual void renderUI(visionaray::pinhole_camera const& cam)
    {
        // store this here, don't have access to it elsewhere..
        this->cam = cam;

        ToolbarUI();

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

        // Rotation
        // if (rotManip == nullptr) {
        //     rotManip = std::make_shared<visionaray::rotate_manipulator>(
        //         cam,
        //         modelMatrix,
        //         visionaray::vec3f(bbox.size().z*.5f),
        //         visionaray::mouse::Left,
        //         3
        //     );
        // }

        // if (doRotate && !rotManip->active()) {

        // }

        // rotManip->set_active(doRotate);

        // if (rotManip->active())
        //      rotManip->render();


        if (doMove) {
            transManip->set_active(doMove);

            if (transManip->active())
                transManip->render();
        }

        ImGui::End();
    }

    virtual void afterRenderUI()
    {
        if (picked.geomHandle != nullptr) {
            renderTriangleGeomPipelineGL(picked.glPipeline,cam.get_view_matrix(),
                                         cam.get_proj_matrix(),modelMatrix);

            if (manip==0) {
                doDelete = doRotate = doMove = false;
            } else if ((manip==2 && !doRotate) || (manip==3 && !doMove)) {
                ASGSurface surf = picked.handle;

                // TODO: might have more than one parents..
                ASGObject parent = nullptr;
                ASG_SAFE_CALL(asgObjectGetParent(surf,0,&parent));

                ASGType_t type;
                ASG_SAFE_CALL(asgGetType(parent,&type));

                ASGTransform trans;

                if (type!=ASG_TYPE_TRANSFORM) {
                    float matrix[] = {1.f,0.f,0.f,
                                      0.f,1.f,0.f,
                                      0.f,0.f,1.f,
                                      0.f,0.f,0.f};

                    ASG_SAFE_CALL(asgRetain(surf));
                    ASG_SAFE_CALL(asgObjectRemoveChild(parent,surf));
                    trans = asgNewTransform(matrix);
                    ASG_SAFE_CALL(asgObjectAddChild(trans,surf));
                    ASG_SAFE_CALL(asgObjectAddChild(parent,trans));

                } else {
                    trans = parent;
                }

                float matrix[12];
                ASG_SAFE_CALL(asgTransformGetMatrix(trans,matrix));

                modelMatrix = visionaray::mat4(visionaray::mat4x3(matrix));

                int numPaths=0;
                asgObjectGetParentPaths(surf,root,NULL,NULL,&numPaths);
                std::vector<int> pathLengths(numPaths);
                int* pl = pathLengths.data();
                asgObjectGetParentPaths(surf,root,NULL,&pl,&numPaths);
                ASGObject** paths = new ASGObject*[numPaths];
                for (int i=0; i<numPaths; ++i) {
                    paths[i] = new ASGObject[pathLengths[i]];
                }
                asgObjectGetParentPaths(surf,root,paths,&pl,&numPaths);
                if (numPaths != 1) {
                    std::cerr << "oups\n"; // TODO!
                }
                picked.parentPath.clear();
                for (int i=0; i<pathLengths[0]; ++i) {
                    picked.parentPath.push_back(paths[0][i]);
                }
                for (int i=0; i<numPaths; ++i) {
                    delete[] paths[i];
                }
                delete[] paths;

                picked.transform = trans;

                if (manip==2) {
                    doRotate = true;
                } else {
                    // Translation
                    {
                        visionaray::aabb bounds;
                        ASGTriangleGeometry geom = picked.geomHandle;
                        ASG_SAFE_CALL(asgGeometryComputeBounds(
                                (ASGGeometry)geom,
                                &bounds.min.x,&bounds.min.y,&bounds.min.z,
                                &bounds.max.x,&bounds.max.y,&bounds.max.z));

                        auto verts = visionaray::compute_vertices(bounds);

                        for (auto it = picked.parentPath.rbegin(); it != picked.parentPath.rend(); ++it) {
                            ASGType_t type;
                            ASG_SAFE_CALL(asgGetType(*it,&type));

                            if (type==ASG_TYPE_TRANSFORM) {
                                float m[12];
                                ASG_SAFE_CALL(asgTransformGetMatrix((ASGTransform)*it,m));
                                visionaray::mat4x3 m4x3(m);

                                for (int i=0; i<8; ++i) {
                                    //verts[i] = (verts[i]*m4x3).xyz();
                                    verts[i] = m4x3*visionaray::vec4f(verts[i],1.f);
                                }
                            }
                        }

                        bounds.invalidate();
                        for (int i=0; i<8; ++i) {
                            bounds.insert(verts[i]);
                        }

                        //std::cout << modelMatrix << '\n';
                        //std::cout << bounds.size() << '\n';
                        //printSceneGraph(root,true);

                        transManip = std::make_shared<visionaray::translate_manipulator>(
                            cam,
                            modelMatrix,
                            visionaray::vec3f(bounds.size()*.0006f), // TODO: largest
                            visionaray::mouse::Left,
                            3
                        );

                        std::cout << bounds.center() << '\n';
                        transManip->set_position(bounds.center());
                    }
                    doMove = true;
                }

            } else if (manip==1 && doDelete) {
                ASGSurface surf = picked.handle;

                // TODO: might have more than one parents..
                ASGObject parent = nullptr;
                ASG_SAFE_CALL(asgObjectGetParent(surf,0,&parent));
                ASG_SAFE_CALL(asgObjectRemoveChild(parent,surf));
                ASG_SAFE_CALL(asgRelease(surf));
                // TODO: release geom etc.
                picked.handle = nullptr;
                picked.geomHandle = nullptr;

                doDelete = false;
                rebuildANARIWorld = true;
            }
        }
    }

    bool needFrameReset()
    {
        return rebuildANARIWorld;
    }

    visionaray::aabb bbox;

    std::vector<ASGMaterial> materials;

    visionaray::pinhole_camera cam;

    std::shared_ptr<visionaray::rotate_manipulator> rotManip = nullptr;
    std::shared_ptr<visionaray::translate_manipulator> transManip = nullptr;
    visionaray::mat4 modelMatrix = visionaray::mat4::identity();

    int toolbarHeight = 48;

    int manip = 0;
    bool doRotate = false;
    bool doMove = false;
    bool doDelete = false;

    struct {
        ASGSurface handle = nullptr;
        ASGTriangleGeometry geomHandle = nullptr;
        TriangleGeomPipelineGL glPipeline;
        visionaray::mouse::pos downPos;
        ASGTransform transform = nullptr; // the parent transform
        std::vector<ASGObject> parentPath;
    } picked;

    bool rebuildANARIWorld = false;
};

// Load scene w/ assimp, ground plane, cameras
struct FilmStudio : Scene
{
    FilmStudio(ANARIDevice device, ANARIWorld wrld, const char* fileName)
        : Scene(device,wrld)
    {
        texture = GLuint(-1);

        bbox.invalidate();

        root = asgNewObject();

        ASGObject model = asgNewObject();

        // Load from file
        ASG_SAFE_CALL(asgLoadASSIMP(model,fileName,0));

        // Compute bounding box
        ASG_SAFE_CALL(asgComputeBounds(model,&bbox.min.x,&bbox.min.y,&bbox.min.z,
                                       &bbox.max.x,&bbox.max.y,&bbox.max.z,0));

        // Add a ground plane that's slightly bigger
        float size = visionaray::max_element(bbox.size());
        size *= 1.5f;
        float size2 = size*.5f;
        visionaray::vec3f center = bbox.center();
        bbox.min = visionaray::vec3f(center.x-size2,bbox.min.y,center.z-size2);
        bbox.max = visionaray::vec3f(center.x+size2,bbox.max.y,center.z+size2);

        static float groundPlaneVertex[] = {bbox.min.x,bbox.min.y,bbox.min.z,
                                            bbox.max.x,bbox.min.y,bbox.min.z,
                                            bbox.max.x,bbox.min.y,bbox.max.z,
                                            bbox.min.x,bbox.min.y,bbox.max.z};

        static uint32_t groundPlaneIndex[] = {0,1,2, 0,2,3};

        ASGTriangleGeometry groundPlaneGeom = asgNewTriangleGeometry(groundPlaneVertex,
                                                                     NULL,NULL,4,
                                                                     groundPlaneIndex,
                                                                     2,NULL);

        ASGSurface groundPlane = asgNewSurface(groundPlaneGeom,NULL);

        float matrix[] = {1.f,0.f,0.f,
                          0.f,1.f,0.f,
                          0.f,0.f,1.f,
                          0.f,0.f,0.f};
        modelTransform = asgNewTransform(matrix);
        ASG_SAFE_CALL(asgObjectAddChild(modelTransform,model));
        ASG_SAFE_CALL(asgObjectAddChild(root,groundPlane));
        ASG_SAFE_CALL(asgObjectAddChild(root,modelTransform));

        visionaray::vec3f camPos = bbox.max - bbox.size()*.1f;
        camTransform1 = asgNewTransform(matrix);
        ASG_SAFE_CALL(asgTransformTranslate(camTransform1,camPos.data()));
        ASG_SAFE_CALL(asgObjectAddChild(root,camTransform1));

        static float vertex[] = {-.5f,-.3f,-.3f,
                                  .5f,-.3f,-.3f,
                                  .5f, .3f,-.3f,
                                 -.5f, .3f,-.3f,
                                  .5f,-.3f, .3f,
                                 -.5f,-.3f, .3f,
                                 -.5f, .3f, .3f,
                                  .5f, .3f, .3f};

        static uint32_t index[] = {0,1,2, 0,2,3, 4,5,6, 4,6,7,
                                   1,4,7, 1,7,2, 5,0,3, 5,3,6,
                                   5,4,1, 5,1,0, 3,2,7, 3,7,6};

        ASGTriangleGeometry boxGeom = asgNewTriangleGeometry(vertex,NULL,NULL,8,
                                                             index,12,NULL);
        // ASGSurface boxSurf = asgNewSurface(boxGeom,NULL);
        // ASG_SAFE_CALL(asgObjectAddChild(camTransform1,boxSurf));

        // Movable camera
        cam1 = asgNewCamera("perspective");
        float pos[] = { 0.f, 0.f, 0.f };
        ASG_SAFE_CALL(asgCameraSetParam(cam1,asgParam3fv("position",pos)));
        float dir[] = { -1.f, 0.f, 0.f };
        ASG_SAFE_CALL(asgCameraSetParam(cam1,asgParam3fv("direction",dir)));
        float up[] = { 0.f, 1.f, 0.f };
        ASG_SAFE_CALL(asgCameraSetParam(cam1,asgParam3fv("up",up)));
        ASG_SAFE_CALL(asgObjectAddChild(camTransform1,cam1));

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);

        // Init cam matrix
        float m[12];
        ASG_SAFE_CALL(asgTransformGetMatrix(camTransform1,m));
        camMat4 = visionaray::mat4(visionaray::mat4x3(m));
    }

    virtual void beforeRenderFrame()
    {
        float axis[3] = { 0,1,0 };
        asgTransformRotate(modelTransform,axis,.002f);

        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_TRANSFORMS,0));

        anariCommit(device,world);

        resetFrame = true;
    }

    virtual void renderUI(visionaray::pinhole_camera const& cam)
    {
        unsigned width = 320, height = 240;

        struct Data {
            ANARIDevice device;
            ASGCamera cam;
            ANARICamera anariCamera;
            float aspect;
            std::vector<visionaray::mat4x3> transStack;
        };
        Data data;
        data.device = device;
        data.cam = cam1;
        data.anariCamera = anariNewCamera(device,"perspective");
        data.aspect = width/(float)height;

        ASGVisitor visitor = asgCreateVisitor(
            [](ASGVisitor self, ASGObject obj, void* userData)
            {
                Data* data = (Data*)userData;
                ASGType_t t;
                asgGetType(obj,&t);

                if (obj==data->cam) {
                    float position[3] {0.f,0.f,0.f};
                    float direction[3] {0.f,0.f,0.f};
                    float up[3] {0.f,0.f,0.f};
                    ASGParam positionParam, directionParam, upParam;
                    ASG_SAFE_CALL(asgCameraGetParam(obj,"position",&positionParam));
                    ASG_SAFE_CALL(asgParamGetValue(positionParam,position));
                    ASG_SAFE_CALL(asgCameraGetParam(obj,"direction",&directionParam));
                    ASG_SAFE_CALL(asgParamGetValue(directionParam,direction));
                    ASG_SAFE_CALL(asgCameraGetParam(obj,"up",&upParam));
                    ASG_SAFE_CALL(asgParamGetValue(upParam,up));

                    for (size_t i=0; i<data->transStack.size(); ++i) {
                        visionaray::vec3f pPos = (data->transStack[i] * visionaray::vec4f(visionaray::vec3f(position),1.f));
                        position[0] = pPos[0]; position[1] = pPos[1]; position[2] = pPos[2];
                        visionaray::vec3f vDir = data->transStack[i] * visionaray::vec4f(visionaray::vec3f(direction),0.f);
                        direction[0] = vDir.x; direction[1] = vDir[1]; direction[2] = vDir[2];
                        visionaray::vec3f vUp = data->transStack[i] * visionaray::vec4f(visionaray::vec3f(up),0.f);
                        up[0] = vUp.x; up[1] = vUp[1]; up[2] = vUp[2];
                    }

                    // for (int i=0; i<3; ++i) std::cout << position[i] << ' ';
                    // std::cout << '\n';

                    // for (int i=0; i<3; ++i) std::cout << direction[i] << ' ';
                    // std::cout << '\n';

                    // for (int i=0; i<3; ++i) std::cout << up[i] << ' ';
                    // std::cout << '\n';
                    // std::cout << '\n';

                    anariSetParameter(data->device, data->anariCamera, "aspect", ANARI_FLOAT32, &data->aspect);
                    anariSetParameter(data->device, data->anariCamera, "position", ANARI_FLOAT32_VEC3, position);
                    anariSetParameter(data->device, data->anariCamera, "direction", ANARI_FLOAT32_VEC3, direction);
                    anariSetParameter(data->device, data->anariCamera, "up", ANARI_FLOAT32_VEC3, up);
                    anariCommit(data->device,data->anariCamera);

                } else if (t==ASG_TYPE_TRANSFORM) {
                    visionaray::mat4x3 m4x3;
                    float* d = m4x3.data();
                    ASG_SAFE_CALL(asgTransformGetMatrix(obj,m4x3.data()));
                    data->transStack.push_back(m4x3);
                    ASG_SAFE_CALL(asgVisitorApply(self,obj));
                    data->transStack.pop_back();
                } else {
                    ASG_SAFE_CALL(asgVisitorApply(self,obj));
                }
            },
            &data,ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN
        );
        ASG_SAFE_CALL(asgVisitorApply(visitor,root));

        // printSceneGraph(root,true);

        if (renderer == nullptr) {
            renderer = anariNewRenderer(device,"default");
            anariCommit(device,renderer);
        }

        if (frame == nullptr) {
            frame = anariNewFrame(device);
            anariSetParameter(device,frame,"world",ANARI_WORLD,&world);
            unsigned imgSize[2] = { width, height };
            anariSetParameter(device,frame,"size",ANARI_UINT32_VEC2,imgSize);
            anariSetParameter(device,frame,"renderer",ANARI_RENDERER,&renderer);
            anariCommit(device,frame);
        }

        if (cameraChanged) {
            anariSetParameter(device,frame,"camera",ANARI_CAMERA,&data.anariCamera);
            anariCommit(device,frame);
        }
        anariRenderFrame(device,frame);
        anariFrameReady(device,frame,ANARI_WAIT);
        const uint32_t *fbPointer = (uint32_t *)anariMapFrame(device, frame, "color");
        ImGui::Begin("Cam1");

        if (texture == GLuint(-1))
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,fbPointer);

        ImGui::ImageButton(
            (void*)(intptr_t)texture,
            ImVec2(width,height),
            ImVec2(0, 1),
            ImVec2(1, 0),
            0 // frame size = 0
            );
        anariUnmapFrame(device, frame, "color");
        ImGui::End();
        if (rotManip == nullptr) {
            rotManip = std::make_shared<visionaray::rotate_manipulator>(
                cam,
                camMat4,
                visionaray::vec3f(bbox.size().z*.1f),
                visionaray::mouse::Left,
                3
            );

            rotManip->set_active(true);
        }

        if (transManip == nullptr) {
            transManip = std::make_shared<visionaray::translate_manipulator>(
                cam,
                camMat4,
                visionaray::vec3f(bbox.size().z*.1f),
                visionaray::mouse::Left,
                3
            );

            //transManip->set_active(true);
        }

        if (rotManip->active())
             rotManip->render();

        if (transManip->active())
            transManip->render();
        // ImGui::Begin("Materials");
        // ImGui::End();
    }

    virtual bool needFrameReset()
    {
        if (resetFrame) {
            resetFrame = false;
            return true;
        }

        return false;
    }

    virtual bool handleMouseDown(visionaray::mouse_event const& event)
    {
        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_down(event)) {
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_down(event)) {
                return true;
            }
        }

        return Scene::handleMouseDown(event);
    }

    virtual bool handleMouseUp(visionaray::mouse_event const& event)
    {
        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_up(event)) {
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_up(event)) {
                return true;
            }
        }

        return Scene::handleMouseUp(event);
    }

    virtual bool handleMouseMove(visionaray::mouse_event const& event)
    {
        visionaray::mat4x3 camMat4x3;
        camMat4x3.col0 = camMat4.col0.xyz();
        camMat4x3.col1 = camMat4.col1.xyz();
        camMat4x3.col2 = camMat4.col2.xyz();
        camMat4x3.col3 = camMat4.col3.xyz();

        if (rotManip && rotManip->active()) {
            if (rotManip->handle_mouse_move(event)) {
                ASG_SAFE_CALL(asgTransformSetMatrix(camTransform1,camMat4x3.data()));
                ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                                 ASG_BUILD_WORLD_FLAG_TRANSFORMS,0));
                cameraChanged = true;
                return true;
            }
        }

        if (transManip && transManip->active()) {
            if (transManip->handle_mouse_move(event)) {
                ASG_SAFE_CALL(asgTransformSetMatrix(camTransform1,camMat4x3.data()));
                ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                                 ASG_BUILD_WORLD_FLAG_TRANSFORMS,0));
                cameraChanged = true;
                return true;
            }
        }

        cameraChanged = false;

        return Scene::handleMouseMove(event);
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    visionaray::aabb bbox;

    bool cameraChanged = true;

    bool resetFrame = false;

    GLuint texture;

    ANARIRenderer renderer = nullptr;
    ANARIFrame frame = nullptr;

    ASGTransform modelTransform;
    ASGTransform camTransform1;
    ASGCamera cam1;

    std::shared_ptr<visionaray::rotate_manipulator> rotManip = nullptr;
    std::shared_ptr<visionaray::translate_manipulator> transManip = nullptr;
    visionaray::mat4 camMat4 = visionaray::mat4::identity();
};


