#include <visionaray/math/math.h>
#include <visionaray/pinhole_camera.h>
#include <common/viewer_glut.h>
#include <common/manip/arcball_manipulator.h>
#include <common/manip/pan_manipulator.h>
#include <common/manip/zoom_manipulator.h>
#include <vkt/LookupTable.hpp>
#include "volkit/src/vkt/TransfuncEditor.hpp"

struct Viewer : visionaray::viewer_glut
{
    visionaray::aabb bbox;
    visionaray::pinhole_camera cam;

    vkt::TransfuncEditor tfe;

    void on_display() {
        tfe.show();
    }
};

int main(int argc, char** argv)
{
    using namespace visionaray;

    Viewer viewer;

    try {
        viewer.init(argc,argv);
    } catch (...) {
        return -1;
    }

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

    float aspect = viewer.width()/(float)viewer.height();

    viewer.cam.perspective(45.f * constants::degrees_to_radians<float>(), aspect, .001f, 1000.f);
    viewer.cam.view_all(viewer.bbox);

    viewer.add_manipulator(std::make_shared<arcball_manipulator>(viewer.cam, mouse::Left));
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Middle));
    // Additional "Alt + LMB" pan manipulator for setups w/o middle mouse button
    viewer.add_manipulator(std::make_shared<pan_manipulator>(viewer.cam, mouse::Left, keyboard::Alt));
    viewer.add_manipulator(std::make_shared<zoom_manipulator>(viewer.cam, mouse::Right));

    viewer.event_loop();
}


