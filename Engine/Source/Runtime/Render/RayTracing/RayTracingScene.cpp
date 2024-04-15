//
// Created by MarvelLi on 2024/3/27.
//

#include "RayTracingScene.h"
#include "SceneProxy/TransformProxy.h"
#include "Render/Core/TypeConvertion.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/VertexData.h"
#include "Render/ViewportInterface.h"
#include "SceneProxy/CameraSceneProxy.h"
#include "SceneProxy/LightSceneProxy.h"

namespace MechEngine::Rendering
{

RayTracingScene::RayTracingScene(Stream& stream, Device& device, ImGuiWindow* InWindow, ViewportInterface* InViewport) noexcept:
stream(stream), device(device), Window(InWindow)
{
	rtAccel = device.create_accel({});

	CameraProxy = luisa::make_unique<CameraSceneProxy>(*this);
	LightProxy = luisa::make_unique<LightSceneProxy>(*this);
	TransformProxy = luisa::make_unique<TransformSceneProxy>(*this);
	StaticMeshProxy = luisa::make_unique<StaticMeshSceneProxy>(*this);

	bindlessArray = device.create_bindless_array(bindless_array_capacity);
	Viewport = InViewport;
}

void RayTracingScene::AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform)
{
	StaticMeshProxy->AddStaticMesh(InMesh, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateStaticMesh(StaticMeshComponent* InMesh)
{
	StaticMeshProxy->UpdateStaticMesh(InMesh);
}


void RayTracingScene::EraseMesh(StaticMeshComponent* InMesh)
{

}

void RayTracingScene::AddCamera(CameraComponent* InCamera, TransformComponent* InTransform)
{
	CameraProxy->AddCamera(InCamera, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateCamera(CameraComponent* InCamera)
{
	CameraProxy->UpdateCamera(InCamera);
}

void RayTracingScene::UpdateTransform(TransformComponent* InTransform)
{
	TransformProxy->UpdateTransform(InTransform);
}

void RayTracingScene::AddLight(LightComponent* InLight, TransformComponent* InTransform)
{
	LightProxy->AddLight(InLight, TransformProxy->AddTransform(InTransform));
}

void RayTracingScene::UpdateLight(LightComponent* InLight)
{
	LightProxy->UpdateLight(InLight);
}

Matrix4d RayTracingScene::GetViewMatrix()
{
	return CameraProxy->GetViewMatrix();
}

Matrix4d RayTracingScene::GetProjectionMatrix()
{
	return CameraProxy->GetProjectionMatrix();
}

void RayTracingScene::UploadData()
{
	auto UpdateBindlessArrayIfDirty = [&]() {
		if(bindlessArray.dirty())
		{
			stream << bindlessArray.update();
		}
	};

	// Make sure static mesh data is uploaded before transform data
	// Because need to allocate instance id from accel
	StaticMeshProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	CameraProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LightProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	// This may also update transform in accel
	TransformProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	stream << rtAccel.build() << synchronize();
}

void RayTracingScene::Render()
{
	if (!MainShader)
		MainShader = luisa::make_unique<Shader2D<>>(device.compile<2>([&]() noexcept {
			// Calc view space cordination, left bottom is (-1, -1), right top is (1, 1). Forwards is +Z
			auto pixel_coord = dispatch_id().xy();
			auto pixel       = make_float2(pixel_coord) + .5f;
			auto Resolution  = make_float2(GetWindosSize());
			auto p           = (pixel * 2.0f - Resolution) / Resolution;
			p *= make_float2(1.f, -1.f);

			auto ray = CameraProxy->GenerateRay(p);
			auto intersection = intersect(ray);

			auto light_data = LightProxy->get_light_data(0);
			auto light_pos  = TransformProxy->get_transform_data(light_data.transform_id)->get_location();

			auto distance_light = intersection.depth;
			auto light_dir = normalize(light_pos - intersection.position_world);
			auto half_dir = normalize(light_dir - make_float3(ray->direction()));
			auto diffuse        = max(dot(intersection.triangle_normal_world, light_dir), 0.f) * light_data.linear_color;
			auto specular       = pow(max(dot(intersection.triangle_normal_world, half_dir), 0.f), 32.f) / (distance_light * distance_light) * light_data.linear_color;
			auto color          = select(make_float3(1.f), make_float3(diffuse + specular), intersection.valid());
			// auto Color = make_float3(p + 1.f, 0.f) * 0.5f;
			frame_buffer()->write(pixel_coord, make_float4(color, 1.f));
		}));

	stream << (*MainShader)().dispatch(GetWindosSize()) << synchronize();
}

ray_tracing_hit RayTracingScene::trace_closest(const Var<Ray>& ray) const noexcept
{
	auto hit = rtAccel->intersect(ray, {});
	return ray_tracing_hit {hit.inst, hit.prim, hit.bary};
}

Bool RayTracingScene::has_hit(const Var<Ray>& ray) const noexcept
{
	return rtAccel->intersect_any(ray, {});
}

ray_intersection RayTracingScene::intersect(const Var<Ray>& ray) const noexcept
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
		it.vertex_normal_world = normalize(triangle_interpolate(bary, v0->normal(), v1->normal(), v2->normal()));
		it.depth = length(p - ray->origin());
		it.back_face = dot(normal_world, ray->direction()) > 0.f;
		// .......
	};
	return it;
}

Float4x4 RayTracingScene::get_instance_transform(Expr<uint> instance_id) const noexcept
{
	return rtAccel->instance_transform(instance_id);
}

Image<float>& RayTracingScene::frame_buffer() const noexcept
{
	return Window->framebuffer();
}

uint2 RayTracingScene::GetWindosSize() const noexcept
{
	return Window->framebuffer().size();
}

}
