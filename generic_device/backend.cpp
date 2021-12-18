#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <thread>
#include <visionaray/math/math.h>
#include <visionaray/bvh.h>
#include <visionaray/texture/texture.h>
#include <visionaray/aligned_vector.h>
#include <visionaray/cpu_buffer_rt.h>
#include <visionaray/generic_material.h>
#include <visionaray/kernels.h>
#include <visionaray/material.h>
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

        struct Frame : render_target
        {
            using SP = std::shared_ptr<Frame>;

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

            thread_pool pool{std::thread::hardware_concurrency()};

            cpu_buffer_rt<PF_RGBA32F, PF_DEPTH32F> accumBuffer;

            bool sRGB = false;

            void* colorPtr = nullptr;
            void* depthPtr = nullptr;

            ANARIFrame handle = nullptr;
        };

        struct Camera
        {
            using SP = std::shared_ptr<Camera>;

            thin_lens_camera impl;
            ANARICamera handle = nullptr;
        };

        struct StructuredVolumeRef
        {
            texture_ref<float, 3> texture3f;
            texture_ref<vec4f, 1> textureRGBA;

            aabb bbox;
            float mu_ = 1.f;

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
        };

        struct StructuredVolume
        {
            using SP = std::shared_ptr<StructuredVolume>;

            void reset(generic::Volume& volume)
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

                    ref.texture3f = texture_ref<float, 3>(storage3f);

                    ref.bbox = aabb({0.f,0.f,0.f},{(float)data->numItems[0],(float)data->numItems[1],(float)data->numItems[2]});

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

                    ref.textureRGBA = texture_ref<vec4f, 1>(storageRGBA);

                    handleRGB = color;
                    handleA = opacity;
                }
            }

            texture<float, 3> storage3f;
            texture<vec4f, 1> storageRGBA;

            StructuredVolumeRef ref;

            ANARIVolume handle = nullptr;
            ANARIArray3D handle3f = nullptr;
            ANARIArray1D handleRGB = nullptr;
            ANARIArray1D handleA = nullptr;
        };

        typedef index_bvh<basic_triangle<3,float>> TriangleBVH;
        typedef index_bvh<typename TriangleBVH::bvh_inst> TriangleTLAS;

        struct TriangleGeom
        {
            using SP = std::shared_ptr<TriangleGeom>;

            TriangleBVH bvh;
            ANARIGeometry handle = nullptr;
            unsigned geomID = unsigned(-1);
        };

        struct Material
        {
            using SP = std::shared_ptr<Material>;

            vec3f color{.8f,.8f,.8f};
            ANARIMaterial handle = nullptr;
        };

        struct Surface
        {
            using SP = std::shared_ptr<Surface>;

            TriangleBVH::bvh_inst bvhInst;
            Material::SP material = nullptr;

            ANARISurface handle = nullptr;
        };

        struct World
        {
            using SP = std::shared_ptr<World>;

            struct {
                TriangleTLAS tlas;
                aligned_vector<TriangleBVH::bvh_inst> bvhInsts;
                aligned_vector<generic_material<matte<float>>> materials;
            } surfaceImpl;

            struct {
                aligned_vector<StructuredVolumeRef*> structuredVolumes;
            } volumeImpl;

            ANARIWorld handle = nullptr;
        };

        struct Renderer
        {
            Algorithm algorithm;
            vec4f backgroundColor;
            tiled_sched<ray> sched{std::thread::hardware_concurrency()};
            unsigned accumID=0;

            void renderFrame(Frame& frame, thin_lens_camera& cam, World& world)
            {
                static unsigned frame_num = 0;
                pixel_sampler::basic_jittered_blend_type<float> blend_params;
                blend_params.spp = 1;
                float alpha = 1.f / ++accumID;
                //float alpha = 1.f / 1.f;
                blend_params.sfactor = alpha;
                blend_params.dfactor = 1.f - alpha;
                auto sparams = make_sched_params(blend_params,cam,frame);

                if (algorithm==Algorithm::Pathtracing) {

                    if (world.volumeImpl.structuredVolumes.size()==1) { // single volume only
                        StructuredVolumeRef& volume = *world.volumeImpl.structuredVolumes[0];

                        float heightf = (float)frame.height();

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
                    } else if (!world.surfaceImpl.bvhInsts.empty()) {
                        auto kparams = make_kernel_params(
                            &world.surfaceImpl.tlas,
                            &world.surfaceImpl.tlas+1,
                            world.surfaceImpl.materials.begin(),
                            10,
                            1e-5f,
                            backgroundColor,
                            vec4f{1.f,1.f,1.f,1.f}
                        );
                        pathtracing::kernel<decltype(kparams)> kernel;
                        kernel.params = kparams;
                        sched.frame(kernel,sparams);
                    }
                } else if (algorithm==Algorithm::AmbientOcclusion) {

                    if (1) { // single volume only
                        StructuredVolumeRef& volume = *world.volumeImpl.structuredVolumes[0];

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

        std::vector<TriangleGeom::SP> triangleGeoms;
        std::vector<Material::SP> materials;
        std::vector<Surface::SP> surfaces;
        std::vector<StructuredVolume::SP> structuredVolumes;

        Renderer renderer;
        std::vector<Camera::SP> cameras;
        std::vector<Frame::SP> frames;
        World world;

        void createDefaultMaterial()
        {
            Material::SP dflt = std::make_shared<Material>();
            materials.push_back(dflt);
        }

        enum class ExecutionOrder {
            Object = 9999, // catch error messages first
            StructuredRegular = 6,
            TriangleGeom = 6,
            Matte = 6,
            Volume = 5,
            Surface = 5,
            World = 4,
            Camera = 3,
            PerspectiveCamera = 2,
            Renderer = 1,
            AO = 1,
            Pathtracer = 1,
            Frame = 0,
        };

        typedef std::function<void()> CommitFunc;

        struct Commit {
            CommitFunc func;
            ExecutionOrder order;
        };

        std::vector<Commit> outstandingCommits;

        static void flushCommitBuffer() {
            std::sort(outstandingCommits.begin(),outstandingCommits.end(),
                      [](const Commit& a, const Commit& b)
                      {
                          return a.order > b.order;
                      });

            for (auto commit : outstandingCommits) {
                commit.func();
            }

            outstandingCommits.clear();
        }

        void enqueueCommit(CommitFunc func, ExecutionOrder order) {
            outstandingCommits.push_back({func,order});
        }
    } // backend

    //--- API ---------------------------------------------
    namespace backend {

        void commit(generic::Object& obj)
        {
            enqueueCommit([]() {
                LOG(logging::Level::Warning) << "Backend: no commit() implementation found";
            }, ExecutionOrder::Object);
        }

        void commit(generic::World& world)
        {
            enqueueCommit([&world]() {
                // Surfaces
                if (world.surface != nullptr) { // TODO: should check if there were any changes at all
                    Array1D* surfaces = (Array1D*)GetResource(world.surface);

                    backend::world.surfaceImpl.bvhInsts.clear();
                    backend::world.surfaceImpl.materials.clear();

                    unsigned defaultMatID = 0;
                    std::vector<Material::SP> mats;
                    mats.push_back(backend::materials[defaultMatID]);

                    for (uint32_t i=0; i<surfaces->numItems[0]; ++i) {
                        ANARISurface surf = ((ANARISurface*)(surfaces->data))[i];
                        auto sit = std::find_if(backend::surfaces.begin(),backend::surfaces.end(),
                                                [surf](const Surface::SP& srf) {
                                                    return srf->handle == surf;
                                                });

                        assert(sit != backend::surfaces.end());

                        unsigned instID = 0;

                        if ((*sit)->material != nullptr) {
                            ANARIMaterial m = (*sit)->material->handle;

                            auto mit = std::find_if(mats.begin(),mats.end(),
                                                    [m](const Material::SP& mat) {
                                                        return mat->handle == m;
                                                    });

                            if (mit != mats.end()) {
                                instID = mit-mats.begin();
                            } else {
                                instID = (unsigned)mats.size();
                                mats.push_back((*sit)->material);
                            }
                        }

                        TriangleBVH::bvh_inst inst = (*sit)->bvhInst;
                        inst.set_inst_id(instID);
                        backend::world.surfaceImpl.bvhInsts.push_back(inst);
                    }

                    lbvh_builder builder;

                    backend::world.surfaceImpl.tlas = builder.build(TriangleTLAS{},
                                                                    backend::world.surfaceImpl.bvhInsts.data(),
                                                                    backend::world.surfaceImpl.bvhInsts.size());

                    for (size_t i=0; i<mats.size(); ++i) {
                        Material::SP m = mats[i];

                        matte<float> mat;
                        mat.ca() = from_rgb(vec3f{1.f,1.f,1.f});
                        mat.cd() = from_rgb(m->color);
                        mat.ka() = 1.f;
                        mat.kd() = 1.f;

                        backend::world.surfaceImpl.materials.push_back(mat);
                    }
                }

                // Volumes
                if (world.volume != nullptr) { // TODO: should check if there were any changes at all
                    Array1D* volumes = (Array1D*)GetResource(world.volume);

                    backend::world.volumeImpl.structuredVolumes.clear();

                    for (uint32_t i=0; i<volumes->numItems[0]; ++i) {
                        ANARIVolume vol = ((ANARIVolume*)(volumes->data))[i];
                        auto vit = std::find_if(backend::structuredVolumes.begin(),backend::structuredVolumes.end(),
                                                [vol](const StructuredVolume::SP& sv) {
                                                    return sv->handle == vol;
                                                });

                        assert(vit != backend::structuredVolumes.end());

                        backend::world.volumeImpl.structuredVolumes.push_back(&(*vit)->ref);
                    }
                }

            }, ExecutionOrder::World);
        }

        void commit(generic::Camera& cam)
        {
            enqueueCommit([&cam]() {
                auto it = std::find_if(backend::cameras.begin(),backend::cameras.end(),
                                       [&cam](const Camera::SP& c) {
                                           return c->handle == cam.getResourceHandle();
                                       });

                if (it == backend::cameras.end()) {
                    Camera::SP c = std::make_shared<Camera>();
                    c->handle = (ANARICamera)cam.getResourceHandle();
                    backend::cameras.push_back(c);
                    it = backend::cameras.end()-1;
                }

                vec3f eye(cam.position);
                vec3f dir(cam.direction);
                vec3f center = eye+dir;
                vec3f up(cam.up);
                if (eye!=(*it)->impl.eye() || center!=(*it)->impl.center() || up!=(*it)->impl.up()) {
                    (*it)->impl.look_at(eye,center,up);
                    (*it)->impl.set_lens_radius(cam.apertureRadius);
                    (*it)->impl.set_focal_distance(cam.focusDistance);
                    renderer.accumID = 0;
                }
            }, ExecutionOrder::Camera);
        }

        void commit(generic::PerspectiveCamera& cam)
        {
            enqueueCommit([&cam]() {
                auto it = std::find_if(backend::cameras.begin(),backend::cameras.end(),
                                       [&cam](const Camera::SP& c) {
                                           return c->handle == cam.getResourceHandle();
                                       });

                if (it == backend::cameras.end()) {
                    Camera::SP c = std::make_shared<Camera>();
                    c->handle = (ANARICamera)cam.getResourceHandle();
                    backend::cameras.push_back(c);
                    it = backend::cameras.end()-1;
                }

                (*it)->impl.perspective(cam.fovy,cam.aspect,.001f,1000.f);
            }, ExecutionOrder::PerspectiveCamera);
        }

        void commit(generic::Frame& frame)
        {
            enqueueCommit([&frame]() {
                auto it = std::find_if(backend::frames.begin(),backend::frames.end(),
                                       [&frame](const Frame::SP& f) {
                                           return f->handle == frame.getResourceHandle();
                                       });

                if (it == backend::frames.end()) {
                    Frame::SP f = std::make_shared<Frame>();
                    f->handle = (ANARIFrame)frame.getResourceHandle();
                    backend::frames.push_back(f);
                    it = backend::frames.end()-1;
                }

                pixel_format color
                    = frame.color==ANARI_UFIXED8_VEC4 || ANARI_UFIXED8_RGBA_SRGB
                        ? PF_RGBA8 : PF_RGBA32F;

                pixel_format depth
                    = frame.color==ANARI_FLOAT32 ? PF_DEPTH32F : PF_UNSPECIFIED;

                bool sRGB = frame.color==ANARI_UFIXED8_RGBA_SRGB;

                (*it)->reset(frame.size[0],frame.size[1],color,depth,sRGB);

                // Also resize camera viewport
                auto cit = std::find_if(backend::cameras.begin(),backend::cameras.end(),
                                        [&frame](const Camera::SP& c) {
                                            return c->handle == frame.camera;
                                        });

                assert(cit != backend::cameras.end());

                (*cit)->impl.set_viewport(0,0,frame.size[0],frame.size[1]);
            }, ExecutionOrder::Frame);
        }

        void commit(generic::TriangleGeom& geom)
        {
            enqueueCommit([&geom]() {
                auto it = std::find_if(backend::triangleGeoms.begin(),backend::triangleGeoms.end(),
                                       [&geom](const TriangleGeom::SP& tg) {
                                           return tg->handle != nullptr
                                               && tg->handle == geom.getResourceHandle();
                                       });

                unsigned geomID(-1);

                if (it == backend::triangleGeoms.end()) {
                    backend::triangleGeoms.push_back(std::make_shared<TriangleGeom>());
                    it = backend::triangleGeoms.end()-1;
                    geomID = backend::triangleGeoms.size()-1;
                } else {
                    geomID = std::distance(it,backend::triangleGeoms.begin());
                }

                (*it)->handle = (ANARIGeometry)geom.getResourceHandle();
                (*it)->geomID = geomID;

                aligned_vector<basic_triangle<3,float>> triangles;

                if (geom.primitive_index != nullptr) {
                    Array1D* vertex = (Array1D*)GetResource(geom.vertex_position);
                    Array1D* index = (Array1D*)GetResource(geom.primitive_index);

                    vec3f* vertices = (vec3f*)vertex->data;
                    vec3ui* indices = (vec3ui*)index->data;

                    triangles.resize(index->numItems[0]);

                    for (uint32_t i=0; i<index->numItems[0]; ++i) {
                        vec3f v1 = vertices[indices[i].x];
                        vec3f v2 = vertices[indices[i].y];
                        vec3f v3 = vertices[indices[i].z];

                        triangles[i].prim_id = i;
                        triangles[i].geom_id = geomID;
                        triangles[i].v1 = v1;
                        triangles[i].e1 = v2-v1;
                        triangles[i].e2 = v3-v1;
                    }

                    binned_sah_builder builder;
                    builder.enable_spatial_splits(true);

                    (*it)->bvh = builder.build(TriangleBVH{},triangles.data(),triangles.size());
                } else {
                    assert(0 && "not implemented yet!!");
                }
            }, ExecutionOrder::TriangleGeom);
        }

        void commit(generic::Matte& mat)
        {
            enqueueCommit([&mat]() {
                if (backend::materials.empty()) {
                    backend::createDefaultMaterial();
                }

                auto it = std::find_if(backend::materials.begin(),backend::materials.end(),
                                       [&mat](const Material::SP& material) {
                                           return material->handle != nullptr
                                               && material->handle == mat.getResourceHandle();
                                       });

                if (it == backend::materials.end()) {
                    Material::SP m = std::make_shared<Material>();
                    m->handle = (ANARIMaterial)mat.getResourceHandle();
                    m->color = vec3f{mat.color};
                    backend::materials.push_back(m);
                } else {
                    (*it)->color = vec3f{mat.color};
                }

                renderer.accumID = 0;
            }, ExecutionOrder::Matte);
        }

        void commit(generic::Surface& surf)
        {
            enqueueCommit([&surf]() {
                auto it = std::find_if(backend::surfaces.begin(),backend::surfaces.end(),
                                       [&surf](const Surface::SP& srf) {
                                           return srf->handle == surf.getResourceHandle();
                                       });

                if (it == backend::surfaces.end()) {
                    backend::surfaces.push_back(std::make_shared<Surface>());
                    it = backend::surfaces.end()-1;
                }

                (*it)->handle = (ANARISurface)surf.getResourceHandle();

                assert(surf.geometry != nullptr);

                auto git = std::find_if(backend::triangleGeoms.begin(),backend::triangleGeoms.end(),
                                        [&surf](const TriangleGeom::SP& tg) {
                                            return tg->handle == surf.geometry;
                                        });

                assert(git != backend::triangleGeoms.end());

                (*it)->bvhInst = (*git)->bvh.inst({mat3x3::identity(),vec3f(0.f)});

                if (surf.material != nullptr) {
                    auto mit = std::find_if(backend::materials.begin(),backend::materials.end(),
                                            [&surf](const Material::SP& mat) {
                                                return mat->handle == surf.material;
                                            });

                    assert(mit != backend::materials.end());

                    (*it)->material = *mit;
                }

                renderer.accumID = 0;
            }, ExecutionOrder::Surface);
        }

        void commit(generic::StructuredRegular& sr)
        {
        }

        void commit(generic::Volume& vol)
        {
            enqueueCommit([&vol]() {
                auto it = std::find_if(backend::structuredVolumes.begin(),
                                       backend::structuredVolumes.end(),
                                       [&vol](const StructuredVolume::SP& sv) {
                                           return sv->handle != nullptr
                                               && sv->handle == vol.getResourceHandle();
                                       });

                if (it == backend::structuredVolumes.end()) {
                    backend::structuredVolumes.push_back(std::make_shared<StructuredVolume>());
                    it = backend::structuredVolumes.end()-1;
                }

                (*it)->handle = (ANARIVolume)vol.getResourceHandle();
                (*it)->reset(vol);

                renderer.accumID = 0;
            }, ExecutionOrder::Volume);
        }

        void commit(generic::Renderer& rend)
        {
            enqueueCommit([&rend]() {
                renderer.backgroundColor = vec4f(rend.backgroundColor);
            }, ExecutionOrder::Renderer);
        }

        void commit(generic::Pathtracer& pt)
        {
            enqueueCommit([&pt]() {
                renderer.algorithm = Algorithm::Pathtracing;
            }, ExecutionOrder::Pathtracer);
        }

        void commit(generic::AO& ao)
        {
            enqueueCommit([&ao]() {
                renderer.algorithm = Algorithm::AmbientOcclusion;
            }, ExecutionOrder::AO);
        }

        void* map(generic::Frame& frame)
        {
            auto it = std::find_if(backend::frames.begin(),backend::frames.end(),
                                   [&frame](const Frame::SP& f) {
                                       return f->handle == frame.getResourceHandle();
                                   });

            assert(it != backend::frames.end());

            return (*it)->colorPtr;
        }

        void renderFrame(generic::Frame& frame)
        {
            flushCommitBuffer();

            frame.renderFuture = std::async([&frame]() {
                auto it = std::find_if(backend::frames.begin(),backend::frames.end(),
                                       [&frame](const Frame::SP& f) {
                                           return f->handle == frame.getResourceHandle();
                                       });

                assert(it != backend::frames.end());

                auto cit = std::find_if(backend::cameras.begin(),backend::cameras.end(),
                                        [&frame](const Camera::SP& c) {
                                            return c->handle == frame.camera;
                                        });

                assert(cit != backend::cameras.end());

                auto start = std::chrono::steady_clock::now();
                renderer.renderFrame(**it,(*cit)->impl,backend::world);
                auto end = std::chrono::steady_clock::now();
                frame.duration = std::chrono::duration<float>(end - start).count();
            });
        }

        int wait(generic::Frame& frame, ANARIWaitMask)
        {
            frame.renderFuture.wait();
            return 1;
        }

    } // backend
} // generic


