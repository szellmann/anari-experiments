#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <random>
#include <vector>
#include <visionaray/math/math.h>
#include <visionaray/pinhole_camera.h>
#include <common/viewer_glut.h>
#include <common/manip/arcball_manipulator.h>
#include <common/manip/pan_manipulator.h>
#include <common/manip/zoom_manipulator.h>
#include <common/model.h>
#include <common/sg.h>
#include <Support/CmdLine.h>
#include <Support/CmdLineUtil.h>
#include <vkt/InputStream.hpp>
#include <vkt/LookupTable.hpp>
#include <vkt/Resample.hpp>
#include <vkt/StructuredVolume.hpp>
#include <vkt/VolumeFile.hpp>
#include "volkit/src/vkt/TransfuncEditor.hpp"
#include <asg/asg.h>
#include <anari/anari_cpp.hpp>
#include <imgui.h>

#define ASG_SAFE_CALL(X) X

static std::string getExt(const std::string &fileName)
{
    int pos = fileName.rfind('.');
    if (pos == fileName.npos)
        return "";
    return fileName.substr(pos);
}


void statusFunc(void *userData,
    ANARIDevice device,
    ANARIObject source,
    ANARIDataType sourceType,
    ANARIStatusSeverity severity,
    ANARIStatusCode code,
    const char *message)
{
    (void)userData;
    if (severity == ANARI_SEVERITY_FATAL_ERROR)
        fprintf(stderr, "[FATAL] %s\n", message);
    else if (severity == ANARI_SEVERITY_ERROR)
        fprintf(stderr, "[ERROR] %s\n", message);
    else if (severity == ANARI_SEVERITY_WARNING)
        fprintf(stderr, "[WARN ] %s\n", message);
    else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING)
        fprintf(stderr, "[PERF ] %s\n", message);
    else if (severity == ANARI_SEVERITY_INFO)
        fprintf(stderr, "[INFO] %s\n", message);
}

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

    ANARIDevice device = nullptr;
    ANARIWorld world = nullptr;

    ASGObject root = nullptr;
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
    }

   ~VolumeScene()
    {
        ASG_SAFE_CALL(asgRelease(volume));
    }

    visionaray::aabb getBounds()
    {
        return {{0,0,0},{(float)volDims[0],(float)volDims[1],(float)volDims[2]}};
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

    ASGStructuredVolume volume = nullptr;
    ASGLookupTable1D lut = nullptr;
    int volDims[3];
    std::vector<float> volData;
    std::vector<float> rgbLUT;
    std::vector<float> alphaLUT;
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
        ASGVisitor visitor = asgCreateVisitor([](ASGObject obj, void* userData) {
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
        },&materials);
        ASG_SAFE_CALL(asgApplyVisitor(root,visitor,
                                      ASG_VISITOR_TRAVERSAL_TYPE_CHILDREN));

        // Build up ANARI world
        ASG_SAFE_CALL(asgBuildANARIWorld(root,device,world,
                                         ASG_BUILD_WORLD_FLAG_FULL_REBUILD,0));

        anariCommit(device,world);
    }

    virtual visionaray::aabb getBounds()
    {
        return bbox;
    }

    visionaray::aabb bbox;

    std::vector<ASGMaterial> materials;
};

struct Viewer : visionaray::viewer_glut
{
    visionaray::pinhole_camera cam;

    vkt::TransfuncEditor tfe;

    std::string fileName;

    Viewer() : viewer_glut(512,512,"ANARI experiments") {

        using namespace support;

        add_cmdline_option(cl::makeOption<std::string&>(
            cl::Parser<>(),
            "filenames",
            cl::Desc("Input files in wavefront obj format"),
            cl::Positional,
            cl::Optional,
            cl::init(fileName)
            ));
    }

    void on_display() {
        // Setup camera for this frame
        float aspect = cam.aspect();
        float pos[3] = { cam.eye().x, cam.eye().y, cam.eye().z };
        float view[3] = { cam.center().x-cam.eye().x,
                          cam.center().y-cam.eye().y,
                          cam.center().z-cam.eye().z };
        float up[3] = { cam.up().x, cam.up().y, cam.up().z };

        ANARICamera camera = anariNewCamera(anari.device,"perspective");
        anariSetParameter(anari.device, camera, "aspect", ANARI_FLOAT32, &aspect);
        anariSetParameter(anari.device, camera, "position", ANARI_FLOAT32_VEC3, &pos);
        anariSetParameter(anari.device, camera, "direction", ANARI_FLOAT32_VEC3, &view);
        anariSetParameter(anari.device, camera, "up", ANARI_FLOAT32_VEC3, &up);
        anariCommit(anari.device, camera);

        if (auto volumeScene = dynamic_cast<VolumeScene*>(anari.scene)) {
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
                    volumeScene->updateLUT(colorVals,alphaVals,dims.x);
                    delete[] alphaVals;
                    delete[] colorVals;
                }
            }
        }

        float bgColor[4] = {.3f, .3f, .3f, 1.f};
        anariSetParameter(anari.device, anari.renderer, "backgroundColor", ANARI_FLOAT32_VEC4, bgColor);
        anariCommit(anari.device, anari.renderer);

        // Prepare frame for rendering
        ANARIFrame frame = anariNewFrame(anari.device);
        unsigned imgSize[2] = { (unsigned)width(), (unsigned)height() };
        anariSetParameter(anari.device, frame, "size", ANARI_UINT32_VEC2, imgSize);
        //ANARIDataType fbFormat = ANARI_UFIXED8_VEC4;
        ANARIDataType fbFormat = ANARI_UFIXED8_RGBA_SRGB;
        anariSetParameter(anari.device, frame, "color", ANARI_DATA_TYPE, &fbFormat);

        anariSetParameter(anari.device, frame, "renderer", ANARI_RENDERER, &anari.renderer);
        anariSetParameter(anari.device, frame, "camera", ANARI_CAMERA, &camera);
        anariSetParameter(anari.device, frame, "world", ANARI_WORLD, &anari.world);
        anariCommit(anari.device, frame);

        int spp=1;
        for (int frames = 0; frames < spp; frames++) {
            anariRenderFrame(anari.device, frame);
            anariFrameReady(anari.device, frame, ANARI_WAIT);
        }

        const uint32_t *fbPointer = (uint32_t *)anariMapFrame(anari.device, frame, "color");
        glClearColor(bgColor[0],bgColor[1],bgColor[2],bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawPixels(width(),height(),GL_RGBA,GL_UNSIGNED_BYTE,fbPointer);
        anariUnmapFrame(anari.device, frame, "color");

        // TODO: keep persistent where appropriate!
        anariRelease(anari.device, camera);
        anariRelease(anari.device, frame);

        if (dynamic_cast<VolumeScene*>(anari.scene)) {
            ImGui::Begin("Volume");
            tfe.drawImmediate();
            ImGui::End();
        } else if (auto model = dynamic_cast<Model*>(anari.scene)) {

            bool rebuildANARIWorld = false;

            ImGui::Begin("Materials");

            static const char* current_item = NULL;
            static ASGMaterial mat = NULL;

            if (ImGui::BeginCombo("##combo", current_item))
            {
                for (size_t m=0; m<model->materials.size(); ++m)
                {
                    const char* name;
                    ASG_SAFE_CALL(asgMaterialGetName(model->materials[m],&name));
                    bool is_selected = (current_item == name);
                    if (ImGui::Selectable(name, is_selected)) {
                        mat = model->materials[m];
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

            if (rebuildANARIWorld) {
                ASG_SAFE_CALL(asgBuildANARIWorld(model->root,model->device,model->world,
                                                 ASG_BUILD_WORLD_FLAG_MATERIALS,0));

                anariCommit(model->device,model->world);
            }
        }
    }

    void on_resize(int w, int h) {
        cam.set_viewport(0, 0, w, h);
        float aspect = w/(float)h;
        cam.perspective(45.f * visionaray::constants::degrees_to_radians<float>(), aspect, .001f, 1000.f);

        viewer_glut::on_resize(w,h);
    }

    struct {
        std::string libType = "environment";
        std::string devType = "default";
        ANARILibrary library = nullptr;
        ANARIDevice device = nullptr;
        ANARIRenderer renderer = nullptr;
        ANARIWorld world = nullptr;
        Scene* scene = nullptr;



        void init(std::string fileName) {
            library = anariLoadLibrary(libType.c_str(), statusFunc);
            if (library == nullptr)
                throw std::runtime_error("Error loading ANARI library");
            ANARIDevice dev = anariNewDevice(library,devType.c_str());
            if (dev == nullptr)
                throw std::runtime_error("Error creating new ANARY device");
            anariCommit(dev,dev);
            device = dev;
            world = anariNewWorld(device);
            if (fileName.empty())
                scene = new VolumeScene(device,world);
            else if (getExt(fileName)==".raw" || getExt(fileName)==".xvf" || getExt(fileName)==".rvf")
                scene = new VolumeScene(device,world,fileName.c_str());
            else
                scene = new Model(device,world,fileName.c_str());

            // Setup renderer
            const char** deviceSubtypes = anariGetDeviceSubtypes(library);
            while (const char* dstype = *deviceSubtypes++) {
                std::cout << "ANARI_RENDERER subtypes supported:\n";
                const char** rendererTypes = anariGetObjectSubtypes(library, dstype, ANARI_RENDERER);
                while (rendererTypes && *rendererTypes) {
                    const char* rendererType = *rendererTypes++;
                    std::cout << rendererType << '\n';
                }

                std::cout << "ANARI_VOLUME subtypes supported:\n";
                const char** volumeTypes = anariGetObjectSubtypes(library, dstype, ANARI_VOLUME);
                while (volumeTypes && *volumeTypes) {
                    const char* volumeType = *volumeTypes++;
                    std::cout << volumeType << '\n';
                }
            }
            renderer = anariNewRenderer(device, "default");
            //renderer = anariNewRenderer(device, "raycast");
            //renderer = anariNewRenderer(device, "ao");

        }

        void release() {
            delete scene;
            anariRelease(device, renderer);
            anariRelease(device, world);
            anariRelease(device,device);
            anariUnloadLibrary(library);
        }
    } anari;
};

int main(int argc, char** argv)
{
    using namespace visionaray;

    // Boilerplate to use visionaray's viewer
    Viewer viewer;

    try {
        viewer.init(argc,argv);
    } catch (...) {
        return -1;
    }

    // Setup ANARI library, device, and renderer
    viewer.anari.init(viewer.fileName);

    // Set up the volkit TFE
    vkt::LookupTable lut; // keep alive throughout main loop
    if (auto volumeScene = dynamic_cast<VolumeScene*>(viewer.anari.scene)) {
        float* rgb;
        float* alpha;
        int32_t numEntries;

        ASG_SAFE_CALL(asgLookupTable1DGetRGB(volumeScene->lut, &rgb));
        ASG_SAFE_CALL(asgLookupTable1DGetAlpha(volumeScene->lut, &alpha));
        ASG_SAFE_CALL(asgLookupTable1DGetNumEntries(volumeScene->lut, &numEntries));

        std::vector<float> rgba(numEntries*4);
        for (int32_t i=0; i<numEntries; ++i) {
            rgba[i*4] = rgb[i*3];
            rgba[i*4+1] = rgb[i*3+1];
            rgba[i*4+2] = rgb[i*3+2];
            rgba[i*4+3] = alpha[i];
        }

        lut = vkt::LookupTable(5,1,1,vkt::ColorFormat::RGBA32F);
        lut.setData((uint8_t*)rgba.data());
        viewer.tfe.setLookupTableResource(lut.getResourceHandle());
    }

    // More boilerplate to set up camera manipulators
    float aspect = viewer.width()/(float)viewer.height();
    viewer.cam.perspective(45.f * constants::degrees_to_radians<float>(), aspect, .001f, 1000.f);
    viewer.cam.view_all(viewer.anari.scene->getBounds());

    viewer.add_manipulator(std::make_shared<arcball_manipulator>(viewer.cam, mouse::Left));
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Middle));
    // Additional "Alt + LMB" pan manipulator for setups w/o middle mouse button
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Left, keyboard::Alt));
    viewer.add_manipulator(std::make_shared<zoom_manipulator>(viewer.cam, mouse::Right));

    // Repeatedly render the scene
    viewer.event_loop();

    viewer.anari.release();
}


