#include <visionaray/cpu_buffer_rt.h>
#include "backend.hpp"
#include "logging.hpp"

using namespace visionaray;

namespace generic {

    namespace backend {
        enum class Algorithm { Pathtracing, };

        struct RenderTarget
        {
            cpu_buffer_rt<PF_RGBA8,   PF_UNSPECIFIED> rgba8_unspecified;
            cpu_buffer_rt<PF_RGBA32F, PF_UNSPECIFIED> rgba32f_unspecified;
            cpu_buffer_rt<PF_RGBA8,   PF_DEPTH32F>    rgba8_depth32f;
            cpu_buffer_rt<PF_RGBA32F, PF_DEPTH32F>    rgba32f_depth32f;

            bool sRGB = false;

            void* colorPtr = nullptr;
            void* depthPtr = nullptr;

            void reset(int width, int height, pixel_format color, pixel_format depth, bool srgb)
            {
                if (color==PF_RGBA8 && depth==PF_UNSPECIFIED) {
                    rgba8_unspecified.resize(width,height);
                    colorPtr = rgba8_unspecified.color();
                    depthPtr = nullptr;
                } else if (color==PF_RGBA8 && depth==PF_DEPTH32F) {
                    rgba8_depth32f.resize(width,height);
                    colorPtr = rgba8_depth32f.color();
                    depthPtr = rgba8_depth32f.depth();
                } else if (color==PF_RGBA32F && depth==PF_UNSPECIFIED) {
                    rgba32f_unspecified.resize(width,height);
                    colorPtr = rgba32f_unspecified.color();
                    depthPtr = nullptr;
                } else if (color==PF_RGBA32F && depth==PF_DEPTH32F) {
                    rgba32f_depth32f.resize(width,height);
                    colorPtr = rgba32f_depth32f.color();
                    depthPtr = rgba32f_depth32f.depth();
                }

                sRGB = srgb;
            }
        };

        struct Renderer
        {
            RenderTarget renderTarget;

            vec4f backgroundColor;
        };


        // State
        Algorithm algorithm;
        Renderer renderer;

    } // backend

    //--- API ---------------------------------------------
    namespace backend {

        void commit(Object& obj)
        {
            LOG(logging::Level::Warning) << "Backend: no commit() implementation found";
        }

        void commit(World& world)
        {
        }

        void commit(StructuredRegular& sr)
        {
        }

        void commit(Frame& frame)
        {
            pixel_format color
                = frame.color==ANARI_UFIXED8_VEC4 || ANARI_UFIXED8_RGBA_SRGB
                    ? PF_RGBA8 : PF_RGBA32F;

            pixel_format depth
                = frame.color==ANARI_FLOAT32 ? PF_DEPTH32F : PF_UNSPECIFIED;

            bool sRGB = frame.color==ANARI_UFIXED8_RGBA_SRGB;

            renderer.renderTarget.reset(frame.size[0],frame.size[1],color,depth,sRGB);
        }

        void commit(Pathtracer& pt)
        {
            algorithm = Algorithm::Pathtracing;
        }

        void* map(Frame& frame)
        {
            return renderer.renderTarget.colorPtr;
        }

        void renderFrame(Frame& frame)
        {
            if (algorithm==Algorithm::Pathtracing) {

            }
        }

    } // backend
} // generic


