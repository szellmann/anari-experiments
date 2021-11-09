#include <iostream>
#include <visionaray/math/math.h>
#include <visionaray/texture/texture.h>
#include <visionaray/aligned_vector.h>
#include <visionaray/cpu_buffer_rt.h>
#include "array.hpp"
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

        struct StructuredVolume
        {
            StructuredVolume() = default;

            void reset(Volume& volume)
            {
                handle = (ANARIVolume)volume.getResourceHandle();

                ANARISpatialField f = volume.field;
                StructuredRegular* sr = (StructuredRegular*)GetResource(f);
                ANARIArray3D d = sr->data;
                if (d != handle3f) { // new volume data!
                    Array3D* data = (Array3D*)GetResource(d);

                    storage3f = texture<float, 3>((unsigned)data->numItems[0],
                                                  (unsigned)data->numItems[1],
                                                  (unsigned)data->numItems[2]);
                    storage3f.reset((const float*)data->data);
                    storage3f.set_filter_mode(Linear);
                    storage3f.set_address_mode(Clamp);

                    texture3f = texture_ref<float, 3>(storage3f);

                    handle3f = d;
                }

                ANARIArray1D color = volume.color;
                ANARIArray1D opacity = volume.opacity;

                if (color != handleRGB || opacity != handleA) { // TF changed
                    Array1D* rgb = (Array1D*)GetResource(color);
                    Array1D* a = (Array1D*)GetResource(opacity);

                    aligned_vector<vec4f> rgba(rgb->numItems[0]);
                    for (uint64_t i = 0; i < rgb->numItems[0]; ++i) {
                        vec4f val(((vec3f*)rgb->data)[i],((float*)a)[i]);
                        rgba[i] = val;
                    }

                    storageRGBA = texture<vec4f, 1>(rgb->numItems[0]);

                    handleRGB = color;
                    handleA = opacity;
                }
            }

            texture<float, 3> storage3f;
            texture_ref<float, 3> texture3f;
            texture<vec4f, 1> storageRGBA;
            texture_ref<vec4f, 1> textureRGBA;

            ANARIVolume handle = nullptr;
            ANARIArray3D handle3f = nullptr;
            ANARIArray1D handleRGB = nullptr;
            ANARIArray1D handleA = nullptr;
        };

        struct Renderer
        {
            Algorithm algorithm;
            RenderTarget renderTarget;
            vec4f backgroundColor;
            aligned_vector<StructuredVolume> structuredVolumes;
        };


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
            if (world.volume != nullptr) {
                Array1D* volumes = (Array1D*)GetResource(world.volume);

                // TODO: count the actual _structured_ volumes, resize after!
                renderer.structuredVolumes.resize(volumes->numItems[0]);

                for (uint64_t i = 0; i < volumes->numItems[0]; ++i) {
                    ANARIVolume vol = ((ANARIVolume*)(volumes->data))[i];
                    Volume* volume = (Volume*)GetResource(vol);
                    renderer.structuredVolumes[i].reset(*volume);
                }
            }
        }

        void commit(StructuredRegular& sr)
        {
        }

        void commit(Volume& vol)
        {
            // only effects the backend if world was already committed,
            for (size_t i = 0; i < renderer.structuredVolumes.size(); ++i) {
                if (renderer.structuredVolumes[i].handle != nullptr
                    && renderer.structuredVolumes[i].handle == vol.getResourceHandle()) {
                    renderer.structuredVolumes[i].reset(vol);
                    break;
                }
            }
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
            renderer.algorithm = Algorithm::Pathtracing;
        }

        void* map(Frame& frame)
        {
            return renderer.renderTarget.colorPtr;
        }

        void renderFrame(Frame& frame)
        {
            if (renderer.algorithm==Algorithm::Pathtracing) {

            }
        }

    } // backend
} // generic


