#include <stdio.h>
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
#include <vkt/LookupTable.hpp>
#include "volkit/src/vkt/TransfuncEditor.hpp"
#include <anari/anari_cpp.hpp>

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

struct Viewer : visionaray::viewer_glut
{
    visionaray::aabb bbox{{0,0,0},{63,63,63}};
    visionaray::pinhole_camera cam;

    vkt::TransfuncEditor tfe;
    vkt::ResourceHandle originalTF;

    Viewer() : viewer_glut(512,512,"ANARI experiments") {}

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

        // Setup volumetric world
        ANARIWorld world = anariNewWorld(anari.device);

        ANARIVolume volume = anariNewVolume(anari.device, "scivis");

        // TODO: make persistent, don't recompute all the time...
        int volDims[] = { 63, 63, 63 };
        std::vector<float> volData(volDims[0]*volDims[1]*volDims[2]);
        
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

                    volData[linearIndex] = (1.f-sinf(M_PI*zz/2.f) + a * (1+rho(sqrtf(xx*xx+yy*yy))))
                                                / (2.f*(1.f+a));
                }
            }
        }
        ANARIArray3D scalar = anariNewArray3D(anari.device, volData.data(), 0, 0, ANARI_FLOAT32,
                                              volDims[0],volDims[1],volDims[2],0,0,0);
        anariCommit(anari.device, scalar);
        ANARISpatialField field = anariNewSpatialField(anari.device, "structuredRegular");
        anariSetParameter(anari.device, field, "data", ANARI_ARRAY3D, &scalar);
        const char* filter = "nearest";
        anariSetParameter(anari.device, field, "filter", ANARI_STRING, filter);
        anariCommit(anari.device, field);

        vkt::LookupTable* lut = tfe.getUpdatedLookupTable();
        if (lut == nullptr)
            lut = (vkt::LookupTable*)vkt::GetManagedResource(originalTF); // not yet updated
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
        ANARIArray1D color = anariNewArray1D(anari.device, colorVals, 0, 0, ANARI_FLOAT32_VEC3, dims.x, 0);
        anariCommit(anari.device, color);

        ANARIArray1D opacity = anariNewArray1D(anari.device, alphaVals, 0, 0, ANARI_FLOAT32, dims.x, 0);
        anariCommit(anari.device, opacity);

        float valueRange[2] = {0.f,1.f};

        anariSetParameter(anari.device, volume, "field", ANARI_SPATIAL_FIELD, &field);
        anariSetParameter(anari.device, volume, "color", ANARI_ARRAY1D, &color);
        anariSetParameter(anari.device, volume, "opacity", ANARI_ARRAY1D, &opacity);
        //anariSetParameter(anari.device, volume, "valueRange", ANARI_FLOAT32_BOX1, &valueRange);
        anariCommit(anari.device, volume);

        ANARIArray1D volumes = anariNewArray1D(anari.device, &volume, 0, 0, ANARI_VOLUME, 1, 0);

        anariSetParameter(anari.device, world, "volume", ANARI_ARRAY1D, &volumes);
        anariRelease(anari.device, volumes);
        anariRelease(anari.device, volume);
        anariRelease(anari.device, field);
        anariRelease(anari.device, color);
        anariRelease(anari.device, opacity);

        anariCommit(anari.device, world);

        // Setup renderer
        ANARIRenderer renderer = anariNewRenderer(anari.device, "default");

        float bgColor[4] = {.3f, .3f, .3f, 0.f};
        anariSetParameter(anari.device, renderer, "backgroundColor", ANARI_FLOAT32_VEC4, bgColor);
        anariCommit(anari.device, renderer);

        // Prepare frame for rendering
        ANARIFrame frame = anariNewFrame(anari.device);
        unsigned imgSize[2] = { (unsigned)width(), (unsigned)height() };
        anariSetParameter(anari.device, frame, "size", ANARI_UINT32_VEC2, imgSize);
        ANARIDataType fbFormat = ANARI_UFIXED8_RGBA_SRGB;
        anariSetParameter(anari.device, frame, "color", ANARI_DATA_TYPE, &fbFormat);

        anariSetParameter(anari.device, frame, "renderer", ANARI_RENDERER, &renderer);
        anariSetParameter(anari.device, frame, "camera", ANARI_CAMERA, &camera);
        anariSetParameter(anari.device, frame, "world", ANARI_WORLD, &world);
        anariCommit(anari.device, frame);

        int spp=1;
        for (int frames = 0; frames < spp; frames++) {
            anariRenderFrame(anari.device, frame);
            anariFrameReady(anari.device, frame, ANARI_WAIT);
        }

        const uint32_t *fbPointer = (uint32_t *)anariMapFrame(anari.device, frame, "color");
        glDrawPixels(width(),height(),GL_RGBA,GL_UNSIGNED_BYTE,fbPointer);

        // TODO: keep persistent where appropriate!
        anariRelease(anari.device, renderer);
        anariRelease(anari.device, camera);
        anariRelease(anari.device, frame);
        anariRelease(anari.device, world);

        tfe.show();
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

        void init() {
            library = anariLoadLibrary(libType.c_str(), statusFunc);
            if (library == nullptr)
                throw std::runtime_error("Error loading ANARI library");
            ANARIDevice dev = anariNewDevice(library,devType.c_str());
            if (dev == nullptr)
                throw std::runtime_error("Error creating new ANARY device");
            anariCommit(dev,dev);
            device = dev;
        }

        void release() {
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

    // Set up the TFE through volkit
    float rgba[] = {
            1.f, 1.f, 1.f, .005f,
            0.f, .1f, .1f, .25f,
            .5f, .5f, .7f, .5f,
            .7f, .7f, .07f, .75f,
            1.f, .3f, .3f, 1.f
            };
    vkt::LookupTable lut(5,1,1,vkt::ColorFormat::RGBA32F);
    lut.setData((uint8_t*)rgba);
    viewer.tfe.setLookupTableResource(lut.getResourceHandle());
    viewer.originalTF = lut.getResourceHandle();

    // More boilerplate to set up camera manipulators
    float aspect = viewer.width()/(float)viewer.height();
    viewer.cam.perspective(45.f * constants::degrees_to_radians<float>(), aspect, .001f, 1000.f);
    viewer.cam.view_all(viewer.bbox);

    viewer.add_manipulator(std::make_shared<arcball_manipulator>(viewer.cam, mouse::Left));
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Middle));
    // Additional "Alt + LMB" pan manipulator for setups w/o middle mouse button
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Left, keyboard::Alt));
    viewer.add_manipulator(std::make_shared<zoom_manipulator>(viewer.cam, mouse::Right));

    // Setup ANARI library and device
    viewer.anari.init();

    // Repeatedly render the scene
    viewer.event_loop();

    viewer.anari.release();
}


