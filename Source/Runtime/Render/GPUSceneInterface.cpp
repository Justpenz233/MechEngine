//
// Created by MarvelLi on 2024/4/25.
//

#include "GPUSceneInterface.h"
#include "Core/ray_tracing_hit.h"
#include "Core/VertexData.h"
#include "Core/view_data.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"

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

    void GPUSceneInterface::CompileShader()
    {
        ViewModePass = luisa::make_unique<Shader2D<uint>>(device.compile<2>([&](UInt ViewMode)
        {
            auto pixel_coord = dispatch_id().xy();
            $switch(ViewMode)
            {
                $case(static_cast<uint>(ViewMode::DepthBuffer))
                {
                    frame_buffer()->write(pixel_coord, make_float4(g_buffer.depth->read(g_buffer.flattend_index(pixel_coord))));
                };
                $case(static_cast<uint>(ViewMode::NormalWorldBuffer))
                {
                    frame_buffer()->write(pixel_coord, g_buffer.normal->read(pixel_coord));
                };
                $case(static_cast<uint>(ViewMode::BaseColorBuffer))
                {
                    frame_buffer()->write(pixel_coord, g_buffer.base_color->read(pixel_coord));
                };
            };
        }));

    	LineProxy->CompileShader();
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

    ray_intersection GPUSceneInterface::intersect(const Var<Ray>& ray, Var<view_data> view) const noexcept
    {
        auto hit = trace_closest(ray);
        ray_intersection it;
        $if(!hit.miss())
        {
            auto instance_index = hit.instance_id;
        	auto instance_id = rtAccel->instance_user_id(instance_index);
            auto TriangleId = hit.primitive_id;
            auto object_transform = get_instance_transform(instance_index);
            auto Tri = StaticMeshProxy->get_triangle(instance_id, TriangleId);
            auto bary = hit.barycentric;
            auto v_buffer = StaticMeshProxy->get_static_mesh_data(instance_id).vertex_id;
            auto v0 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i0);
            auto v1 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i1);
            auto v2 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i2);

            auto p0_local = v0->position();
            auto p1_local = v1->position();
            auto p2_local = v2->position();

            auto p0_world = (object_transform * make_float4(p0_local, 1.f)).xyz();
            auto p1_world = (object_transform * make_float4(p1_local, 1.f)).xyz();
            auto p2_world = (object_transform * make_float4(p2_local, 1.f)).xyz();

            auto dp0_local = p1_local - p0_local;
            auto dp1_local = p2_local - p0_local;

            auto m = make_float3x3(object_transform);
            auto t = make_float3(object_transform[3]);
            auto p = m * triangle_interpolate(bary, p0_local, p1_local, p2_local) + t;

            auto c = cross(m * dp0_local, m * dp1_local);
            auto normal_world = normalize(c);

            it.vertex_ndc[0] = view->world_to_ndc(p0_world);
            it.vertex_ndc[1] = view->world_to_ndc(p1_world);
            it.vertex_ndc[2] = view->world_to_ndc(p2_world);
            it.instace_id = instance_id;
            it.primitive_id = TriangleId;
            it.position_world = p;
            it.triangle_normal_world = normal_world;
            it.vertex_normal_world = normalize(m * normalize(triangle_interpolate(bary, v0->normal(), v1->normal(), v2->normal())));
        	Float3 cornel_normal[3] = {StaticMeshProxy->get_corner_normal(instance_id, TriangleId, 0),
									   StaticMeshProxy->get_corner_normal(instance_id, TriangleId, 1),
									   StaticMeshProxy->get_corner_normal(instance_id, TriangleId, 2)};
        	it.cornerl_normal_world = normalize(m * normalize(triangle_interpolate(bary, cornel_normal[0], cornel_normal[1], cornel_normal[2])));
            it.depth = view->world_to_ndc(p).z;
            it.back_face = dot(normal_world, ray->direction()) > 0.f;
            it.material_id = StaticMeshProxy->get_static_mesh_data(instance_id).material_id;
            // .......
        };
        return it;
    }

    Float4x4 GPUSceneInterface::get_instance_transform(Expr<uint> instance_id) const noexcept
    {
        return rtAccel->instance_transform(instance_id);
    }

    Var<transform_data> GPUSceneInterface::get_transform(Expr<uint> transform_id) const noexcept
    {
        return TransformProxy->get_transform_data(transform_id);
    }

    Var<Triangle> GPUSceneInterface::get_triangle(const UInt& instance_id, const UInt& triangle_index) const
    {
        return StaticMeshProxy->get_triangle(instance_id, triangle_index);
    }

    Var<Vertex> GPUSceneInterface::get_vertex(const UInt& instance_id, const UInt& vertex_index) const
    {
        return StaticMeshProxy->get_vertex(instance_id, vertex_index);
    }
}
