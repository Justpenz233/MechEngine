//
// Created by MarvelLi on 2024/3/27.
//

#include "GpuScene.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/RayCastHit.h"
#include "Render/Core/VertexData.h"
#include "Render/ViewportInterface.h"
#include "Render/material/shading_function.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/sampler/independent_sampler.h"
#include "Render/sampler/sobol.h"
#include "Render/sampler/sampler_base.h"
#include "rasterizer/scanline_rasterizer.h"
#include "Render/PipeLine/ground_grid/ground_pass.h"
#include "RenderPass/buffer_view_pass.h"
#include "wireframe/wireframe_pass.h"
namespace MechEngine::Rendering
{

GpuScene::GpuScene(Stream& stream, Device& device, ImGuiWindow* InWindow, ViewportInterface* InViewport) noexcept
	: GpuSceneInterface(stream, device), Window(InWindow)
{
	CameraProxy = luisa::make_unique<CameraSceneProxy>(*this);
	LightProxy = luisa::make_unique<LightSceneProxy>(*this);
	TransformProxy = luisa::make_unique<TransformSceneProxy>(*this);
	StaticMeshProxy = luisa::make_unique<StaticMeshSceneProxy>(*this);
	MaterialProxy = luisa::make_unique<MaterialSceneProxy>(*this);
	LineProxy = luisa::make_unique<LineSceneProxy>(*this);
	ShapeProxy = luisa::make_unique<ShapeSceneProxy>(*this);
	Viewport = InViewport;
}

GpuScene::~GpuScene() {}

void GpuScene::UploadRenderData()
{
	auto UpdateBindlessArrayIfDirty = [&]() {
		if (bindlessArray.dirty())
		{
			stream << bindlessArray.update();
		}
	};

	// Make sure static mesh data is uploaded before transform data
	// Because need to allocate instance id from accel
	ShapeProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	StaticMeshProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	CameraProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	MaterialProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LightProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	// This may also update transform in accel
	TransformProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LineProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	if (rtAccel.dirty())
		stream << rtAccel.build() << synchronize();
}

void GpuScene::Render()
{
	CommandList CmdList{};
	PrePass(CmdList);
	CmdList << (*MainShader)(FrameCounter++, TimeCounter++).dispatch(GetWindosSize());
	PostPass(CmdList);
	stream << CmdList.commit();
}

void GpuScene::PostPass(CommandList& CmdList)
{
	if (ViewMode != ViewMode::FrameBuffer) [[unlikely]]
		BufferViewPass->PostPass(CmdList);
	else
	{
		LineProxy->PostRenderPass(CmdList);
		if (!bHDR)
			CmdList << (*ToneMappingPass)().dispatch(GetWindosSize());
	}
	WireFramePass->PostPass(CmdList);
	BufferViewPass->PostPass(CmdList);
	GroundPass->PostPass(CmdList);
	stream << CmdList.commit();
}

ImageView<float> GpuScene::frame_buffer() noexcept
{
	return Window->framebuffer();
}

uint2 GpuScene::GetWindosSize() const noexcept
{
	return Window->framebuffer().size();
}

Var<RayCastHit> GpuScene::trace_closest(const Var<Ray>& ray) const noexcept
{
	auto hit = rtAccel->intersect(ray, {});
	return  {hit.inst, hit.prim, hit.bary};
}

Bool GpuScene::has_hit(const Var<Ray>& ray) const noexcept
{
	return rtAccel->intersect_any(ray, {});
}

ray_intersection GpuScene::intersect(Var<Ray> ray) const noexcept
{
	auto hit = trace_closest(ray);
	return intersect(hit, ray);
}

ray_intersection GpuScene::intersect(Var<RayCastHit> hit, Var<Ray> ray) const noexcept
{
	ray_intersection it;
	$if(!hit->miss())
	{
		it.instance_id = hit.instance_id;
		it.shape = ShapeProxy->get_instance_shape(hit.instance_id);
		auto mesh_id = it.shape->mesh_id;
		auto TriangleId = hit.primitive_id;
		auto object_transform = get_instance_transform(it.instance_id);
		auto Tri = StaticMeshProxy->get_triangle(mesh_id, TriangleId);
		auto bary = hit.barycentric;
		auto v_buffer = StaticMeshProxy->get_static_mesh_data(mesh_id).vertex_buffer_id;
		auto v0 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i0);
		auto v1 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i1);
		auto v2 = bindlessArray->buffer<Vertex>(v_buffer).read(Tri.i2);

		auto p0_local = v0->position();
		auto p1_local = v1->position();
		auto p2_local = v2->position();

		auto dp0_local = p1_local - p0_local;
		auto dp1_local = p2_local - p0_local;

		auto m = make_float3x3(object_transform);
		auto t = make_float3(object_transform[3]);
		auto p = m * triangle_interpolate(bary, p0_local, p1_local, p2_local) + t;

		auto c = cross(m * dp0_local, m * dp1_local);
		auto normal_world = normalize(c);

		it.primitive_id = TriangleId;
		it.position_world = p;
		it.barycentric = bary;
		it.uv = triangle_interpolate(bary, v0->uv(), v1->uv(), v2->uv());
		it.motion_vector = motion_vector(it);

		it.triangle_normal_world = normal_world;
		it.vertex_normal_world = normalize(m * normalize(triangle_interpolate(bary, v0->normal(), v1->normal(), v2->normal())));
		Float3 cornel_normal[3] = { StaticMeshProxy->get_corner_normal(mesh_id, TriangleId, 0),
			StaticMeshProxy->get_corner_normal(mesh_id, TriangleId, 1),
			StaticMeshProxy->get_corner_normal(mesh_id, TriangleId, 2) };
		it.corner_normal_world = normalize(m * normalize(triangle_interpolate(bary, cornel_normal[0], cornel_normal[1], cornel_normal[2])));
		it.depth = dot(p - ray->origin(), ray->direction());
		it.back_face = dot(normal_world, ray->direction()) > 0.f;
		it.material_id = StaticMeshProxy->get_static_mesh_data(mesh_id).material_id;
		// .......
	};
	return it;
}

RayCastHit GpuScene::RayCastQuery(uint PixelX, uint PixelY)
{
	vector<uint2> PixelCoords(MaxQueryCount);
	vector<RayCastHit> Hits(MaxQueryCount);
	PixelCoords[0] = make_uint2(PixelX, PixelY);
	CommandList CmdList{};
	CmdList << RayCastQueryBuffer.copy_from(PixelCoords.data());
	CmdList << (*RayCastQueryShader)().dispatch(1);
	CmdList << RayCastHitBuffer.copy_to(Hits.data());
	stream << CmdList.commit() << synchronize();
	return Hits[0];
}

TArray<RayCastHit> GpuScene::RayCastQuery(const TArray<uint2>& PixelCoords)
{
	TArray<RayCastHit> Hits(MaxQueryCount);
	CommandList CmdList{};
	CmdList << RayCastQueryBuffer.copy_from(PixelCoords.data());
	CmdList << (*RayCastQueryShader)().dispatch(PixelCoords.size());
	CmdList << RayCastHitBuffer.copy_to(Hits.data());
	stream << CmdList.commit() << synchronize();
	Hits.resize(PixelCoords.size());
	return Hits;
}

Float4x4 GpuScene::get_instance_transform(Expr<uint> instance_index) const noexcept
{
	return rtAccel->instance_transform(instance_index);
}

Var<transform_data> GpuScene::get_transform(Expr<uint> transform_id) const noexcept
{
	return TransformProxy->get_transform_data(transform_id);
}

Var<Triangle> GpuScene::get_triangle(const UInt& instance_id, const UInt& triangle_index) const
{
	return StaticMeshProxy->get_triangle(instance_id, triangle_index);
}

Var<Vertex> GpuScene::get_vertex(const UInt& instance_id, const UInt& vertex_index) const
{
	return StaticMeshProxy->get_vertex(instance_id, vertex_index);
}

Float2 GpuScene::motion_vector(const ray_intersection& intersection) const
{
	auto transform_data = TransformProxy->get_instance_transform_data(intersection.instance_id);
	auto& x = intersection.position_world;
	auto& pre_t = transform_data->last_transform_matrix;;
	auto& t_inv = transform_data->inverse_transform_matrix;

	auto pre_x = pre_t * t_inv * make_float4(x, 1.f);
	auto pre_clip_position = CameraProxy->get_main_view().last_view_projection_matrix * pre_x;
	auto pre_ndc_position = (pre_clip_position / pre_clip_position.w).xyz();

	return CameraProxy->get_main_view()->ndc_to_pixel(pre_ndc_position);
}

void GpuScene::CompileShader()
{
	LineProxy->CompileShader();

	BufferViewPass->CompileShader(device, bShaderDebugInfo);

	Rasterizer = make_unique<scanline_rasterizer>(this);
	Rasterizer->CompileShader(device, bShaderDebugInfo);

	// Main pass shader
	MainShader = luisa::make_unique<Shader2D<uint, uint>>(device.compile<2>(
		[&](UInt frame_index, UInt time) noexcept {
			render_main_view(frame_index, time);
		}, ShaderOption{.enable_debug_info = bShaderDebugInfo, .name = "MainShader"}));

	ToneMappingPass = luisa::make_unique<Shader2D<>>(device.compile<2>(
		[&]() noexcept {
			auto pixel_coord = dispatch_id().xy();
			auto color = frame_buffer()->read(pixel_coord);
			if (bHDR)
				frame_buffer()->write(pixel_coord, make_float4(acescg_to_srgb(color.xyz()), 1.f)); // Still need fix
			else
				frame_buffer()->write(pixel_coord, make_float4(linear_to_srgb(acescg_to_srgb(tone_mapping_aces(color.xyz()))), 1.f));
		}, ShaderOption{.enable_debug_info = bShaderDebugInfo, .name = "ToneMappingShader"}));

	// Ray cast query shader
	RayCastQueryBuffer = RegisterBuffer<uint2>(MaxQueryCount);
	RayCastHitBuffer = RegisterBuffer<RayCastHit>(MaxQueryCount);
	RayCastQueryShader = luisa::make_unique<decltype(RayCastQueryShader)::element_type>(device.compile<1>(
		[&]() noexcept {
			auto view = CameraProxy->get_main_view();
			auto query_id = dispatch_id().x;
			auto pixel_coord = RayCastQueryBuffer->read(query_id);
			auto pixel_pos = make_float2(pixel_coord) + 0.5f;
			auto ray = view->generate_ray(pixel_pos);
			auto hit = trace_closest(ray);
			RayCastHitBuffer->write(query_id, hit);
		}, ShaderOption{.enable_debug_info = bShaderDebugInfo, .name = "RayCastQueryShader"}));

	GroundPass = make_unique<ground_pass>(this, GetWindosSize(), frame_buffer());
	GroundPass->CompileShader(device, bShaderDebugInfo);

	WireFramePass = make_unique<wireframe_pass>(*this);
	WireFramePass->CompileShader(device, bShaderDebugInfo);
}

void GpuScene::Init()
{
	GpuSceneInterface::Init();

	LoadRenderSettings();

	CommandList CmdList{};
	InitPass(CmdList);
	stream << CmdList.commit() << synchronize();

	CompileShader();
}


void GpuScene::InitPass(CommandList& CmdList)
{
	// Initialize the sampler
	sampler = luisa::make_unique<independent_sampler>(this, stream);
	stream << synchronize();

	// Compile base shaders
	g_buffer.InitBuffer(device, GetWindosSize());

	BufferViewPass = make_unique<buffer_view_pass>(*this);
	BufferViewPass->InitPass(device, CmdList);
}


} // namespace MechEngine::Rendering
