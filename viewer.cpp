#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <visionaray/math/math.h>
#include <visionaray/pinhole_camera.h>
#include <common/viewer_glut.h>
#include <common/manip/arcball_manipulator.h>
#include <common/manip/pan_manipulator.h>
#include <common/manip/zoom_manipulator.h>
#include <Support/CmdLine.h>
#include <Support/CmdLineUtil.h>
#include <asg/asg.h>
#include <anari/anari_cpp.hpp>
#include <imgui.h>
#include "scenes/grabber.h"
#include "scenes.h"
#include "util.h"


void statusFunc(const void *userData,
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
    visionaray::pinhole_camera cam;

    std::string fileName;

    visionaray::vec4f imageRegion{0.f,0.f,1.f,1.f};

    Viewer() : viewer_glut(512,512,"ANARI experiments") {

        using namespace support;

        set_background_color({.3f,.3f,.3f});

        add_cmdline_option(cl::makeOption<std::string&>(
            cl::Parser<>(),
            "filenames",
            cl::Desc("Input files in wavefront obj format"),
            cl::Positional,
            cl::Optional,
            cl::init(fileName)
            ));

        add_cmdline_option( cl::makeOption<visionaray::vec4&, cl::ScalarType>(
            [&](StringRef name, StringRef /*arg*/, visionaray::vec4& value)
            {
                cl::Parser<>()(name + "-lox", cmd_line_inst().bump(), value.x);
                cl::Parser<>()(name + "-loy", cmd_line_inst().bump(), value.y);
                cl::Parser<>()(name + "-hix", cmd_line_inst().bump(), value.z);
                cl::Parser<>()(name + "-hiy", cmd_line_inst().bump(), value.w);
            },
            "imageRegion",
            cl::Desc("Region of the sensor in normalized screen-space coordinates"),
            cl::ArgDisallowed,
            cl::init(imageRegion)
            ) );
    }

    void resetANARICamera() {
        float aspect = cam.aspect() / ((imageRegion.z-imageRegion.x)/(imageRegion.w-imageRegion.y));;
        float pos[3] = { cam.eye().x, cam.eye().y, cam.eye().z };
        float view[3] = { cam.center().x-cam.eye().x,
                          cam.center().y-cam.eye().y,
                          cam.center().z-cam.eye().z };
        float up[3] = { cam.up().x, cam.up().y, cam.up().z };

        anariSetParameter(anari.device, anari.camera, "aspect", ANARI_FLOAT32, &aspect);
        anariSetParameter(anari.device, anari.camera, "position", ANARI_FLOAT32_VEC3, &pos);
        anariSetParameter(anari.device, anari.camera, "direction", ANARI_FLOAT32_VEC3, &view);
        anariSetParameter(anari.device, anari.camera, "up", ANARI_FLOAT32_VEC3, &up);
        anariSetParameter(anari.device, anari.camera, "imageRegion", ANARI_FLOAT32_BOX2, imageRegion.data());
        anariCommitParameters(anari.device, anari.camera);
    }

    void resetANARIMainLight() {
        float bounds[6];

        static bool reshape = true;
        static visionaray::aabb bbox;

        if (reshape) {
            if (anari.deviceSubtype != std::string("generic")) {
                anariGetProperty(anari.device, anari.world, "bounds", ANARI_FLOAT32_BOX3, &bounds, sizeof(bounds), ANARI_WAIT);
            } else {
                // get bounds property not implemented yet!
                asgComputeBounds(anari.scene->root,&bounds[0],&bounds[1],&bounds[2],
                                 &bounds[3],&bounds[4],&bounds[5]);
            }

            bbox = visionaray::aabb(visionaray::vec3(bounds),visionaray::vec3(bounds+3));

            float intensity = 60.f;
            float radius = 0.f;
            if (anari.deviceSubtype != std::string("generic")) {
                anariSetParameter(anari.device, anari.headLight, "radiance", ANARI_FLOAT32, &intensity);
                radius = length(bbox.max-bbox.min)*.1f;
            } else {
                anariSetParameter(anari.device, anari.headLight, "intensity", ANARI_FLOAT32, &intensity);
                radius = length(bbox.max-bbox.min)*.1f;
            }
            visionaray::vec3f color{.9f,.85f,.85f};
            anariSetParameter(anari.device, anari.headLight, "radius", ANARI_FLOAT32, &radius);
            anariSetParameter(anari.device, anari.headLight, "color", ANARI_FLOAT32_VEC3, color.data());
            if (radius > 0.f) {
                bool visible = false;
                anariSetParameter(anari.device, anari.headLight, "visible", ANARI_BOOL, &visible);
            }

            // compute this only once
            reshape = false;
        }

        bool fixed = false;
        visionaray::vec3 pos = fixed ? bbox.max : cam.eye()+cam.up()*.2f*length(bbox.max-bbox.min);
        anariSetParameter(anari.device, anari.headLight, "position", ANARI_FLOAT32_VEC3, pos.data());

        anariCommitParameters(anari.device, anari.headLight);
        anariCommitParameters(anari.device, anari.world);
    }

    void on_display() {

        float duration = 0.f;
        if (anariGetProperty(anari.device, anari.frame, "duration", ANARI_FLOAT32, &duration, sizeof(duration), ANARI_NO_WAIT)) {
            std::stringstream str;
            str << std::setprecision(2);
            str << std::fixed;
            str << 1.f/duration << " FPS";
            std::string fpsStr = str.str();
            set_window_title(fpsStr.c_str());
        }

        anari.scene->beforeRenderFrame();

        int spp=1;
        for (int frames = 0; frames < spp; frames++) {
            anariRenderFrame(anari.device, anari.frame);
            anariFrameReady(anari.device, anari.frame, ANARI_WAIT);
        }

        bool debugDepth = false;

        if (!debugDepth) {
            uint32_t widthOUT;
            uint32_t heightOUT;
            ANARIDataType typeOUT;
            const uint32_t *fbPointer = (uint32_t *)anariMapFrame(anari.device, anari.frame, "channel.color", &widthOUT, &heightOUT, &typeOUT);
            visionaray::vec4f bgColor(background_color(),1.f);
            glClearColor(bgColor[0],bgColor[1],bgColor[2],bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawPixels(width(),height(),GL_RGBA,GL_UNSIGNED_BYTE,fbPointer);
            anariUnmapFrame(anari.device, anari.frame, "channel.color");
        } else {
            uint32_t widthOUT;
            uint32_t heightOUT;
            ANARIDataType typeOUT;
            const float *dbPointer = (float *)anariMapFrame(anari.device, anari.frame, "channel.depth", &widthOUT, &heightOUT, &typeOUT);
            float *cpy = (float *)malloc(width()*height()*sizeof(float));
            memcpy(cpy,dbPointer,width()*height()*sizeof(float));
            float max = 0.f;
            for (int i=0; i<width()*height(); ++i) {
                if (!std::isinf(cpy[i]))
                    max = std::max(max,cpy[i]);
            }
            for (int i=0; i<width()*height(); ++i) {
                cpy[i] /= max;
            }
            glClear(GL_DEPTH_BUFFER_BIT);
            glDrawPixels(width(),height(),GL_LUMINANCE,GL_FLOAT,cpy);
            anariUnmapFrame(anari.device, anari.frame, "channel.depth");
        }

        anari.scene->afterRenderFrame();

        anari.scene->renderUI(cam);

        anari.scene->afterRenderUI();

        if (anari.scene->needFrameReset())
            anariCommitParameters(anari.device,anari.camera); // provoke frame reset
    }

    void on_resize(int w, int h) {
        visionaray::vec4f bgColor(background_color(),1.f);
        anariSetParameter(anari.device, anari.renderer, "backgroundColor", ANARI_FLOAT32_VEC4, bgColor.data());
        anariCommitParameters(anari.device, anari.renderer);

        cam.set_viewport(0, 0, w, h);
        float aspect = w/(float)h;
        cam.perspective(M_PI/3.f, aspect, .001f, 1000.f);

        unsigned imgSize[2] = { (unsigned)w, (unsigned)h };
        anariSetParameter(anari.device, anari.frame, "size", ANARI_UINT32_VEC2, imgSize);
        anariCommitParameters(anari.device, anari.frame);

        resetANARICamera();
        resetANARIMainLight();

        if (!anari.scene->handleResize(w,h))
            viewer_glut::on_resize(w,h);
    }

    void on_mouse_down(const visionaray::mouse_event& event) {
        if (!anari.scene->handleMouseDown(event))
            viewer_glut::on_mouse_down(event);
    }

    void on_mouse_up(const visionaray::mouse_event& event) {
        if (!anari.scene->handleMouseUp(event))
            viewer_glut::on_mouse_up(event);
    }

    void on_mouse_move(const visionaray::mouse_event& event) {
        if (event.buttons() != visionaray::mouse::button::NoButton) {
            resetANARICamera();
            resetANARIMainLight();
        }
        if (!anari.scene->handleMouseMove(event))
            viewer_glut::on_mouse_move(event);
    }

    void on_space_mouse_move(visionaray::space_mouse_event const& event) {
        if (event.buttons() != visionaray::mouse::button::NoButton) {
            resetANARICamera();
            resetANARIMainLight();
        }
        //if (!anari.scene->handleSpaceMouseMove(event))
            viewer_glut::on_space_mouse_move(event);
    }

    struct {
        std::string libType = "environment";
        std::string devType = "default";
        ANARILibrary library = nullptr;
        ANARIDevice device = nullptr;
        ANARIRenderer renderer = nullptr;
        ANARIWorld world = nullptr;
        ANARILight headLight = nullptr;
        ANARICamera camera = nullptr;
        ANARIFrame frame = nullptr;
        Scene* scene = nullptr;
        std::string deviceSubtype;



        void init(Viewer &instance) {
            std::string fileName = instance.fileName;
            library = anariLoadLibrary(libType.c_str(), statusFunc);
            if (library == nullptr)
                throw std::runtime_error("Error loading ANARI library");
            ANARIDevice dev = anariNewDevice(library,devType.c_str());
            if (dev == nullptr)
                throw std::runtime_error("Error creating new ANARY device");
            anariCommitParameters(dev,dev);
            device = dev;
            world = anariNewWorld(device);
            if (fileName.empty() || fileName=="volume-test")
                scene = new VolumeScene(device,world);
            else if (fileName=="sphere-test")
                scene = new SphereTest(device,world);
            else if (fileName=="select-test")
                scene = new SelectTest(device,world);
            else if (fileName=="grabber")
                scene = new GrabberGame(device,world);
            else if (getExt(fileName)==".raw" || getExt(fileName)==".xvf" || getExt(fileName)==".rvf")
                scene = new VolumeScene(device,world,fileName.c_str());
            else
                scene = new Model(device,world,fileName.c_str());

            headLight = anariNewLight(device,"point");
            ANARIArray1D lights = anariNewArray1D(device,&headLight,0,0,ANARI_LIGHT,1,0);
            anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lights);
            anariCommitParameters(device, world);

            // Setup renderer
            const char** deviceSubtypes = anariGetDeviceSubtypes(library);
            if (deviceSubtypes != nullptr) {
                while (const char* dstype = *deviceSubtypes++) {
                    if (deviceSubtype.empty()) deviceSubtype = std::string(dstype);
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
            }
            renderer = anariNewRenderer(device, "default");
            //int aoSamples = 0;
            //anariSetParameter(device, renderer, "aoSamples", ANARI_INT32, &aoSamples);
            frame = anariNewFrame(device);
            anariSetParameter(device, frame, "world", ANARI_WORLD, &world);
            anariCommitParameters(device, frame);
            //ANARIDataType fbFormat = ANARI_UFIXED8_VEC4;
            ANARIDataType fbFormat = ANARI_UFIXED8_RGBA_SRGB;
            ANARIDataType dbFormat = ANARI_FLOAT32;
            anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &fbFormat);
            //anariSetParameter(device, frame, "depth", ANARI_DATA_TYPE, &dbFormat);
            anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &renderer);
            //renderer = anariNewRenderer(device, "raycast");
            //renderer = anariNewRenderer(device, "ao");
            camera = anariNewCamera(device,"perspective");
            // Prepare frame for rendering
            anariSetParameter(device, frame, "camera", ANARI_CAMERA, &camera);
            anariCommitParameters(device, frame);

        }

        void release() {
            delete scene;
            anariRelease(device,frame);
            anariRelease(device,camera);
            anariRelease(device,headLight);
            anariRelease(device,world);
            anariRelease(device,renderer);
            anariRelease(device,world);
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
    viewer.anari.init(viewer);

    // More boilerplate to set up camera manipulators
    float aspect = viewer.width()/(float)viewer.height();
    viewer.cam.perspective(M_PI/3.f, aspect, .001f, 1000.f);
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


