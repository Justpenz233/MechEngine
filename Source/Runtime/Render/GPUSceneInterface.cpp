//
// Created by MarvelLi on 2024/4/25.
//

#include "GPUSceneInterface.h"
#include "Core/ray_tracing_hit.h"
#include "Core/VertexData.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"

namespace MechEngine::Rendering
{
    GPUSceneInterface::GPUSceneInterface(Stream& stream, Device& device) noexcept
    : stream(stream), device(device)
    {
        rtAccel = device.create_accel({});
        bindlessArray = device.create_bindless_array(bindless_array_capacity);
        // !Proxies should be created in the derived class, case the proxies may be polymorphic for different renderers
    }

    // empty detor here to make unqiue ptr happy
    GPUSceneInterface::~GPUSceneInterface()
    {
    }

    ray_tracing_hit GPUSceneInterface::trace_closest(const Var<Ray>& ray) const noexcept
    {
        auto hit = rtAccel->intersect(ray, {});
        return ray_tracing_hit {hit.inst, hit.prim, hit.bary};
    }

    Bool GPUSceneInterface::has_hit(const Var<Ray>& ray) const noexcept
    {
        return rtAccel->intersect_any(ray, {});
    }

    ray_intersection GPUSceneInterface::intersect(const Var<Ray>& ray) const noexcept
    {
        auto hit = trace_closest(ray);
        ray_intersection it;
        $if(!hit.miss())
        {
            auto InstanceId = hit.instance_id;
            auto TriangleId = hit.primitive_id;
            auto ToWorldTransform = get_instance_transform(InstanceId);
            auto Tri = StaticMeshProxy->get_triangle(InstanceId, TriangleId);
            auto bary = hit.barycentric;
            auto v_buffer = StaticMeshProxy->get_static_mesh_data(InstanceId).vertex_id;
            auto v0 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i0);
            auto v1 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i1);
            auto v2 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i2);

            auto p0_local = v0->position();
            auto p1_local = v1->position();
            auto p2_local = v2->position();

            auto dp0_local = p1_local - p0_local;
            auto dp1_local = p2_local - p0_local;

            auto m = make_float3x3(ToWorldTransform);
            auto t = make_float3(ToWorldTransform[3]);
            auto p = m * triangle_interpolate(bary, p0_local, p1_local, p2_local) + t;


            auto c = cross(m * dp0_local, m * dp1_local);
            auto normal_world = normalize(c);


            it.instace_id = InstanceId;
            it.primitive_id = TriangleId;
            it.position_world = p;
            it.triangle_normal_world = normal_world;
            it.vertex_normal_local = normalize(triangle_interpolate(bary, v0->normal(), v1->normal(), v2->normal()));
            it.vertex_normal_world = normalize(m * it.vertex_normal_local);
            it.depth = length(p - ray->origin());
            it.back_face = dot(normal_world, ray->direction()) > 0.f;
            it.material_id = StaticMeshProxy->get_static_mesh_data(InstanceId).material_id;
            // .......
        };
        return it;
    }

    Float4x4 GPUSceneInterface::get_instance_transform(Expr<uint> instance_id) const noexcept
    {
        return rtAccel->instance_transform(instance_id);
    }

    Var<transformData> GPUSceneInterface::get_transform(Expr<uint> transform_id) const noexcept
    {
        return TransformProxy->get_transform_data(transform_id);
    }
}
