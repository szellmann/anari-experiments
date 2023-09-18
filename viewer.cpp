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
#include "hdri.h"


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

// Helper functions ///////////////////////////////////////////////////////////

static uint32_t cvt_uint32(const float &f)
{
  return static_cast<uint32_t>(255.f * visionaray::clamp(f, 0.f, 1.f));
}

static uint32_t cvt_uint32(const visionaray::vec4 &v)
{
  return (cvt_uint32(v.x) << 0) | (cvt_uint32(v.y) << 8)
      | (cvt_uint32(v.z) << 16) | (cvt_uint32(v.w) << 24);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
static visionaray::vec3 aces_approx(visionaray::vec3 v)
{
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v*(a*v+b))/(v*(c*v+d)+e), visionaray::vec3(0.0f), visionaray::vec3(1.0f));
}

static uint32_t cvt_uint32_aces_approx(const visionaray::vec4 &v)
{
  return cvt_uint32(visionaray::vec4(aces_approx(v.xyz()), v.w));
}

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
static const visionaray::mat3 aces_input_matrix = transpose(visionaray::mat3{
  {0.59719f, 0.35458f, 0.04823f},
  {0.07600f, 0.90834f, 0.01566f},
  {0.02840f, 0.13383f, 0.83777f}
});

static const visionaray::mat3 aces_output_matrix = transpose(visionaray::mat3{
  { 1.60475f, -0.53108f, -0.07367f},
  {-0.10208f,  1.10813f, -0.00605f},
  {-0.00327f, -0.07276f,  1.07602f}
});

static visionaray::vec3 rtt_and_odt_fit(visionaray::vec3 v)
{
  visionaray::vec3 a = v * (v + 0.0245786f) - 0.000090537f;
  visionaray::vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
  return a / b;
}

visionaray::vec3 aces_fitted(visionaray::vec3 v)
{
  v = aces_input_matrix * v;
  v = rtt_and_odt_fit(v);
  return aces_output_matrix * v;
}

static uint32_t cvt_uint32_aces(const visionaray::vec4 &v)
{
  return cvt_uint32(visionaray::vec4(aces_fitted(v.xyz()), v.w));
}

struct Viewer : visionaray::viewer_glut
{
    visionaray::pinhole_camera cam;

    std::string fileName;

    visionaray::vec4f imageRegion{0.f,0.f,1.f,1.f};

    enum Tonemapping {
        Clamped,
        sRGB,
        ACES,
        ACES_Approx,
    } tonemapping = ACES;

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

        add_cmdline_option( cl::makeOption<std::string&>(
            cl::Parser<>(),
            "l",
            cl::Desc("ANARI library"),
            cl::ArgRequired,
            cl::init(anari.libType)
            ) );

        add_cmdline_option( cl::makeOption<std::string&>(
            cl::Parser<>(),
            "hdri",
            cl::Desc("HDRI file"),
            cl::ArgRequired,
            cl::init(anari.hdri)
            ) );

        add_cmdline_option( cl::makeOption<Tonemapping&>({
                { "clamped",     Clamped,     "description: clamped" },
                { "sRGB",        sRGB,        "description: sRGB" },
                { "aces",        ACES,        "description: aces" },
                { "aces-approx", ACES_Approx, "description: aces-approx" }
            },
            "tonemapping",
            cl::Desc("Tone mapping operator"),
            cl::ArgRequired,
            cl::init(tonemapping)
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
        if (!anari.headLight)
            return;

        float bounds[6];

        static visionaray::aabb bbox;

        bool fixed = false;
        visionaray::vec3 pos = fixed ? bbox.max : cam.eye()+cam.up()*.2f*length(bbox.max-bbox.min);
        auto dir = -normalize(pos);
        anariSetParameter(anari.device, anari.headLight, "direction", ANARI_FLOAT32_VEC3, dir.data());

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
            const void *fb = anariMapFrame(anari.device, anari.frame, "channel.color", &widthOUT, &heightOUT, &typeOUT);
            void *fbPointer{nullptr};
            if (tonemapping == ACES) {
                fbPointer = new uint32_t[widthOUT*heightOUT];
                const float *fPtr = (const float *)fb;
                uint32_t *uPtr = (uint32_t *)fbPointer;
                for (size_t i=0; i<widthOUT*size_t(heightOUT); ++i) {
                    visionaray::vec4 color(
                        fPtr[i*4], fPtr[i*4+1], fPtr[i*4+2], fPtr[i*4+3]
                    );
                    uPtr[i] = cvt_uint32_aces(color);
                }
            } else if (tonemapping == ACES_Approx) {
                fbPointer = new uint32_t[widthOUT*heightOUT];
                const float *fPtr = (const float *)fb;
                uint32_t *uPtr = (uint32_t *)fbPointer;
                for (size_t i=0; i<widthOUT*size_t(heightOUT); ++i) {
                    visionaray::vec4 color(
                        fPtr[i*4], fPtr[i*4+1], fPtr[i*4+2], fPtr[i*4+3]
                    );
                    uPtr[i] = cvt_uint32_aces_approx(color);
                }
            } else {
                fbPointer = (void *)fb;
            }
            visionaray::vec4f bgColor(background_color(),1.f);
            glClearColor(bgColor[0],bgColor[1],bgColor[2],bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawPixels(width(),height(),GL_RGBA,GL_UNSIGNED_BYTE,fbPointer);
            anariUnmapFrame(anari.device, anari.frame, "channel.color");
            if (fbPointer != fb) {
                delete[] (uint32_t *)fbPointer;
            }
        } else {
            uint32_t widthOUT;
            uint32_t heightOUT;
            ANARIDataType typeOUT;
            const float *dbPointer = (float *)anariMapFrame(anari.device, anari.frame, "channel.depth", &widthOUT, &heightOUT, &typeOUT);
            float *cpy = (float *)malloc(width()*height()*anari::sizeOf(typeOUT));
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
            free(cpy);
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
        anariSetParameter(anari.device, anari.renderer, "background", ANARI_FLOAT32_VEC4, bgColor.data());
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
        ANARILight envLight = nullptr;
        ANARICamera camera = nullptr;
        ANARIFrame frame = nullptr;
        Scene* scene = nullptr;
        std::string deviceSubtype;
        std::string hdri;



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
            else if (fileName=="amr-test")
                scene = new AMRScene(device,world);
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

            if (hdri.empty()) {
              headLight = anariNewLight(device,"directional");
              ANARIArray1D lights = anariNewArray1D(device,&headLight,0,0,ANARI_LIGHT,1);
              anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lights);
            } else {
              HDRI envMap;
              envMap.load(hdri);
              std::vector<float> pixel(envMap.width*envMap.height*3);
              if (envMap.numComponents == 3) {
                memcpy(pixel.data(), envMap.pixel.data(),
                       envMap.width * envMap.height * 3 * sizeof(float));
              } else if (envMap.numComponents == 4) {
                for (size_t i=0; i<envMap.pixel.size(); i+=4) {
                    size_t i3 = i/4*3;
                    pixel[i3] = envMap.pixel[i];
                    pixel[i3+1] = envMap.pixel[i+1];
                    pixel[i3+2] = envMap.pixel[i+2];
                }
              }
              ANARIArray2D radiance = anariNewArray2D(device,pixel.data(),0,0,
                                                      ANARI_FLOAT32_VEC3,
                                                      envMap.width,envMap.height);

              envLight = anariNewLight(device,"hdri");
              anariSetParameter(device, envLight, "radiance", ANARI_ARRAY2D, &radiance);
              bool visible = true;
              anariSetParameter(device, envLight, "visible", ANARI_BOOL, &visible);
              anariCommitParameters(device, envLight);

              anariRelease(device, radiance);

              ANARIArray1D lights = anariNewArray1D(device,&envLight,0,0,ANARI_LIGHT,1);
              anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lights);
            }
            anariCommitParameters(device, world);

            // Setup renderer
            const char** deviceSubtypes = anariGetDeviceSubtypes(library);
            if (deviceSubtypes != nullptr) {
                while (const char* dstype = *deviceSubtypes++) {
                    if (deviceSubtype.empty()) deviceSubtype = std::string(dstype);
                    std::cout << "ANARI_RENDERER subtypes supported:\n";
                    const char** rendererTypes = anariGetObjectSubtypes(device, ANARI_RENDERER);
                    while (rendererTypes && *rendererTypes) {
                        const char* rendererType = *rendererTypes++;
                        std::cout << rendererType << '\n';
                    }

                    std::cout << "ANARI_VOLUME subtypes supported:\n";
                    const char** volumeTypes = anariGetObjectSubtypes(device, ANARI_VOLUME);
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
            ANARIDataType fbFormat{ANARI_UFIXED8_VEC4};
            if (instance.tonemapping == sRGB)
                fbFormat = ANARI_UFIXED8_RGBA_SRGB;
            else if (instance.tonemapping == ACES || instance.tonemapping == ACES_Approx)
                fbFormat = ANARI_FLOAT32_VEC4;
            ANARIDataType dbFormat = ANARI_FLOAT32;
            anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &fbFormat);
            //anariSetParameter(device, frame, "channel.depth", ANARI_DATA_TYPE, &dbFormat);
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


