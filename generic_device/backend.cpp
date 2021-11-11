#include <iostream>
#include <thread>
#include <visionaray/math/math.h>
#include <visionaray/texture/texture.h>
#include <visionaray/aligned_vector.h>
#include <visionaray/cpu_buffer_rt.h>
#include <visionaray/phase_function.h>
#include <visionaray/random_generator.h>
#include <visionaray/sampling.h>
#include <visionaray/scheduler.h>
#include <visionaray/thin_lens_camera.h>
#include "array.hpp"
#include "backend.hpp"
#include "logging.hpp"

using namespace visionaray;

namespace generic {

    namespace backend {
        enum class Algorithm { Pathtracing, AmbientOcclusion, };

        struct RenderTarget : render_target
        {
            auto ref() { return accumBuffer.ref(); }

            void clear_color_buffer(const vec4f& color = vec4f(0.f))
            {
                accumBuffer.clear_color_buffer(color);
            }

            void begin_frame()
            {
                accumBuffer.begin_frame();
            }

            void end_frame()
            {
                accumBuffer.end_frame();

                parallel_for(pool,tiled_range2d<int>(0,width(),64,0,height(),64),
                    [&](range2d<int> r) {
                        for (int y=r.cols().begin(); y!=r.cols().end(); ++y) {
                            for (int x=r.rows().begin(); x!=r.rows().end(); ++x) {
                                 vec4f src = accumBuffer.color()[y*width()+x];

                                 if (1) {
                                     src.xyz() = linear_to_srgb(src.xyz());
                                     int32_t r = clamp(int32_t(src.x*256.f),0,255);
                                     int32_t g = clamp(int32_t(src.y*256.f),0,255);
                                     int32_t b = clamp(int32_t(src.z*256.f),0,255);
                                     int32_t a = clamp(int32_t(src.w*256.f),0,255);
                                     uint32_t &dst = ((uint32_t*)colorPtr)[y*width()+x];

                                     dst = (r<<0) + (g<<8) + (b<<16) + (a<<24);
                                 }
                            }
                        }
                    });
            }

            void resize(int w, int h)
            {
                render_target::resize(w,h);accumBuffer.resize(w,h);
            }

            thread_pool pool{std::thread::hardware_concurrency()};

            cpu_buffer_rt<PF_RGBA32F, PF_DEPTH32F> accumBuffer;

            bool sRGB = false;

            void* colorPtr = nullptr;
            void* depthPtr = nullptr;

            void reset(int width, int height, pixel_format color, pixel_format depth, bool srgb)
            {
                resize(width,height);

                if (color==PF_RGBA8 && depth==PF_UNSPECIFIED) { // TODO: use anari types!
                    colorPtr = new uint32_t[width*height]; // TODO: delete!!!
                } else if (color==PF_RGBA8 && depth==PF_DEPTH32F) {
                    colorPtr = new uint32_t[width*height];
                    depthPtr = new float[width*height];
                } else if (color==PF_RGBA32F && depth==PF_UNSPECIFIED) {
                    colorPtr = new vec4f[width*height];
                } else if (color==PF_RGBA32F && depth==PF_DEPTH32F) {
                    colorPtr = new vec4f[width*height];
                    depthPtr = new float[width*height];
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

                    bbox = aabb({0.f,0.f,0.f},{(float)data->numItems[0],(float)data->numItems[1],(float)data->numItems[2]});

                    handle3f = d;
                }

                ANARIArray1D color = volume.color;
                ANARIArray1D opacity = volume.opacity;

                if (color != handleRGB || opacity != handleA) { // TF changed
                    Array1D* rgb = (Array1D*)GetResource(color);
                    Array1D* a = (Array1D*)GetResource(opacity);

                    aligned_vector<vec4f> rgba(rgb->numItems[0]);
                    for (uint64_t i = 0; i < rgb->numItems[0]; ++i) {
                        vec4f val(((vec3f*)rgb->data)[i],((float*)a->data)[i]);
                        rgba[i] = val;
                    }

                    storageRGBA = texture<vec4f, 1>(rgb->numItems[0]);
                    storageRGBA.reset(rgba.data());
                    storageRGBA.set_filter_mode(Linear);
                    storageRGBA.set_address_mode(Clamp);

                    textureRGBA = texture_ref<vec4f, 1>(storageRGBA);

                    handleRGB = color;
                    handleA = opacity;
                }
            }

            VSNRAY_FUNC
            vec3 albedo(vec3 const& pos)
            {
                float voxel = (float)tex3D(texture3f, pos / bbox.size());

                // normalize to [0..1]
                //voxel = normalize(volume, voxel);

                vec4f rgba = tex1D(textureRGBA, voxel);
                return rgba.xyz();
            }

            VSNRAY_FUNC
            float mu(vec3 const& pos)
            {
                float voxel = (float)tex3D(texture3f, pos / bbox.size());

                // normalize to [0..1]
                //voxel = normalize(volume, voxel);

                vec4f rgba = tex1D(textureRGBA, voxel);
                return rgba.w;
            }

            template <typename Ray>
            VSNRAY_FUNC
            bool sample_interaction(Ray& r, float d, random_generator<float>& gen)
            {
                float t = 0.0f;
                vec3 pos;

                do
                {
                    t -= log(1.0f - gen.next()) / mu_;

                    pos = r.ori + r.dir * t;
                    if (t >= d)
                    {
                        return false;
                    }
                }
                while (mu(pos) < gen.next() * mu_);

                r.ori = pos;
                return true;
            }

            VSNRAY_FUNC
            inline vec3f gradient(vec3f texCoord, vec3f delta)
            {
                return vec3f(+tex3D(texture3f,vec3f(texCoord.x+delta.x,texCoord.y,texCoord.z))
                             -tex3D(texture3f,vec3f(texCoord.x-delta.x,texCoord.y,texCoord.z)),
                             +tex3D(texture3f,vec3f(texCoord.x,texCoord.y+delta.y,texCoord.z))
                             -tex3D(texture3f,vec3f(texCoord.x,texCoord.y-delta.y,texCoord.z)),
                             +tex3D(texture3f,vec3f(texCoord.x,texCoord.y,texCoord.z+delta.z))
                             -tex3D(texture3f,vec3f(texCoord.x,texCoord.y,texCoord.z-delta.z)));
            }

            texture<float, 3> storage3f;
            texture_ref<float, 3> texture3f;
            texture<vec4f, 1> storageRGBA;
            texture_ref<vec4f, 1> textureRGBA;

            aabb bbox;
            float mu_ = 1.f;

            ANARIVolume handle = nullptr;
            ANARIArray3D handle3f = nullptr;
            ANARIArray1D handleRGB = nullptr;
            ANARIArray1D handleA = nullptr;
        };

        struct Renderer
        {
            Algorithm algorithm;
            RenderTarget renderTarget;
            thin_lens_camera cam;
            vec4f backgroundColor;
            tiled_sched<ray> sched{std::thread::hardware_concurrency()};
            aligned_vector<StructuredVolume> structuredVolumes;
            unsigned accumID=0;

            void renderFrame()
            {
                static unsigned frame_num = 0;
                pixel_sampler::basic_jittered_blend_type<float> blend_params;
                blend_params.spp = 1;
                float alpha = 1.f / ++accumID;
                //float alpha = 1.f / 1.f;
                blend_params.sfactor = alpha;
                blend_params.dfactor = 1.f - alpha;
                auto sparams = make_sched_params(blend_params,cam,renderTarget);

                if (algorithm==Algorithm::Pathtracing) {

                    if (1) { // single volume only
                        StructuredVolume& volume = structuredVolumes[0];

                        float heightf = (float)renderTarget.height();

                        sched.frame([&](ray r, random_generator<float>& gen, int x, int y) {
                            result_record<float> result;
                            
                            henyey_greenstein<float> f;
                            f.g = 0.f; // isotropic

                            vec3f throughput(1.f);

                            auto hit_rec = intersect(r, volume.bbox);

                            unsigned bounce = 0;

                            if (hit_rec.hit)
                            {
                                r.ori += r.dir * hit_rec.tnear;
                                hit_rec.tfar -= hit_rec.tnear;

                                while (volume.sample_interaction(r, hit_rec.tfar, gen))
                                {
                                    // Is the path length exceeded?
                                    if (bounce++ >= 1024)
                                    {
                                        throughput = vec3(0.0f);
                                        break;
                                    }

                                    throughput *= volume.albedo(r.ori);

                                    // Russian roulette absorption
                                    float prob = max_element(throughput);
                                    if (prob < 0.2f)
                                    {
                                        if (gen.next() > prob)
                                        {
                                            throughput = vec3(0.0f);
                                            break;
                                        }
                                        throughput /= prob;
                                    }

                                    // Sample phase function
                                    vec3 scatter_dir;
                                    float pdf;
                                    f.sample(-r.dir, scatter_dir, pdf, gen);
                                    r.dir = scatter_dir;

                                    hit_rec = intersect(r, volume.bbox);
                                }
                            }

                            // Look up the environment
                            float t = y / heightf;
                            vec3 Ld = (1.0f - t)*vec3f(1.0f, 1.0f, 1.0f) + t * vec3f(0.5f, 0.7f, 1.0f);
                            vec3 L = throughput;

                            result.hit = hit_rec.hit;
                            result.color = bounce ? vec4f(L, 1.f) : backgroundColor;
                            return result;
                        }, sparams);
                    }
                } else if (algorithm==Algorithm::AmbientOcclusion) {

                    if (1) { // single volume only
                        StructuredVolume& volume = structuredVolumes[0];

                        float dt = 2.f;
                        vec3f gradientDelta = 1.f/volume.bbox.size();
                        bool volumetricAO = true;

                        sched.frame([&](ray r, random_generator<float>& gen, int x, int y) {
                            result_record<float> result;

                            auto hit_rec = intersect(r, volume.bbox);
                            result.hit = hit_rec.hit;

                            if (!hit_rec.hit) {
                                result.color = backgroundColor;
                                return result;
                            }

                            result.color = vec4f(0.f);

                            float t = max(0.f,hit_rec.tnear);
                            float tmax = hit_rec.tfar;
                            vec3f pos = r.ori + r.dir * t;

                            // TODO: vertex-centric!
                            vec3f texCoord = pos/volume.bbox.size();

                            vec3f inc = r.dir*dt/volume.bbox.size();

                            while (t < tmax) {
                                float voxel = tex3D(volume.texture3f,texCoord);
                                vec4f color = tex1D(volume.textureRGBA,voxel);

                                // shading
                                vec3f grad = volume.gradient(texCoord,gradientDelta);
                                if (volumetricAO && length(grad) > .15f) {
                                    vec3f n = normalize(grad);
                                    n = faceforward(n,-r.dir,n);
                                    vec3f u, v, w=n;
                                    make_orthonormal_basis(u,v,w);

                                    float radius = 1.f;
                                    int numSamples = 2;
                                    for (int i=0; i<numSamples; ++i) {
                                        auto sp = cosine_sample_hemisphere(gen.next(),gen.next());
                                        vec3f dir = normalize(sp.x*u+sp.y*v+sp.z+w);

                                        ray aoRay;
                                        aoRay.ori = texCoord + dir * 1e-3f;
                                        aoRay.dir = dir;
                                        aoRay.tmin = 0.f;
                                        aoRay.tmax = radius;
                                        auto ao_rec = intersect(aoRay,volume.bbox);
                                        aoRay.tmax = fminf(aoRay.tmax,ao_rec.tfar);

                                        vec3f texCoordAO = aoRay.ori/volume.bbox.size();
                                        vec3f incAO = aoRay.dir*dt/volume.bbox.size();

                                        float tAO = 0.f;
                                        while (tAO < aoRay.tmax) {
                                            float voxelAO = tex3D(volume.texture3f,texCoordAO);
                                            vec4f colorAO = tex1D(volume.textureRGBA,voxelAO);

                                            color *= colorAO.w;

                                            texCoordAO += incAO;
                                            tAO += dt;
                                        }
                                    }
                                }

                                // opacity correction
                                color.w = 1.f-powf(1.f-color.w,dt);

                                // premultiplied alpha
                                color.xyz() *= color.w;

                                // compositing
                                result.color += color * (1.f-result.color.w);

                                // step on
                                texCoord += inc;
                                t += dt;
                            }

                            result.color.xyz() += (1.f-result.color.w) * backgroundColor.xyz();
                            result.color.w += (1.f-result.color.w) * backgroundColor.w;

                            return result;
                        }, sparams);
                    }
                }
            }
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

            renderer.cam.set_viewport(0,0,frame.size[0],frame.size[1]);
        }

        void commit(Camera& cam)
        {
            vec3f eye(cam.position);
            vec3f dir(cam.direction);
            vec3f center = eye+dir;
            vec3f up(cam.up);
            if (eye!=renderer.cam.eye() || center!=renderer.cam.center() || up!=renderer.cam.up()) {
                renderer.cam.look_at(eye,center,up);
                renderer.cam.set_lens_radius(cam.apertureRadius);
                renderer.cam.set_focal_distance(cam.focusDistance);
                renderer.accumID = 0;
            }
        }

        void commit(PerspectiveCamera& cam)
        {
            renderer.cam.perspective(cam.fovy,cam.aspect,.001f,1000.f);
        }

        void commit(::generic::Renderer& rend)
        {
            renderer.backgroundColor = vec4f(rend.backgroundColor);
        }

        void commit(Pathtracer& pt)
        {
            renderer.algorithm = Algorithm::Pathtracing;
        }

        void commit(AO& ao)
        {
            renderer.algorithm = Algorithm::AmbientOcclusion;
        }

        void* map(Frame& frame)
        {
            return renderer.renderTarget.colorPtr;
        }

        void renderFrame(Frame& frame)
        {
            frame.renderFuture = std::async([]() {
                renderer.renderFrame();
            });
        }

        int wait(Frame& frame, ANARIWaitMask)
        {
            frame.renderFuture.wait();
            return 1;
        }

    } // backend
} // generic


