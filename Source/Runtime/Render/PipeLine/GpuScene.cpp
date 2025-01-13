//
// Created by MarvelLi on 2024/3/27.
//

#include "GpuScene.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/ray_tracing_hit.h"
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
		if(bindlessArray.dirty())
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

	if (rtAccel.dirty()) stream << rtAccel.build() << synchronize();
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
		CmdList << (*ViewModePass)(static_cast<uint>(ViewMode)).dispatch(GetWindosSize());
	else
	{
		LineProxy->PostRenderPass(CmdList);
		CmdList << (*ToneMappingPass)().dispatch(GetWindosSize());
	}
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

ray_tracing_hit GpuScene::trace_closest(const Var<Ray>& ray) const noexcept
{
	auto hit = rtAccel->intersect(ray, {});
	return ray_tracing_hit {hit.inst, hit.prim, hit.bary};
}

Bool GpuScene::has_hit(const Var<Ray>& ray) const noexcept
{
	return rtAccel->intersect_any(ray, {});
}

ray_intersection GpuScene::intersect(const Var<Ray>& ray) const noexcept
{
	auto hit = trace_closest(ray);
	return intersect(hit, ray);
}

ray_intersection GpuScene::intersect(const ray_tracing_hit& hit, const Var<Ray>& ray) const noexcept
{
	ray_intersection it;
	$if(!hit.miss())
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
		it.shading_frame = frame::make(it.corner_normal_world);
		// .......
	};
	return it;
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

	return CameraProxy->get_main_view()->ndc_to_pixel(pre_ndc_position) - 0.5f;
}

void GpuScene::CompileShader()
{
	// Compile base shaders
	GpuSceneInterface::CompileShader();

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


}

void GpuScene::Init()
{
	GpuSceneInterface::Init();

	LoadRenderSettings();

	InitBuffers();
	InitSamplers();

	CompileShader();
}
void GpuScene::InitBuffers()
{
	auto size = Window->framebuffer().size();
	LOG_DEBUG("GBuffer initial size: {} {}", size.x, size.y);
	g_buffer.base_color = device.create_image<float>(PixelStorage::BYTE4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.normal = device.create_image<float>(PixelStorage::FLOAT4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.depth = device.create_buffer<float>(size.x * size.y);
	g_buffer.instance_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.motion_vector = device.create_image<float>(PixelStorage::FLOAT2,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.frame_buffer = &Window->framebuffer();
	LOG_INFO("Init render frame buffer: {} {}", Window->framebuffer().size().x, Window->framebuffer().size().y);
}



void GpuScene::InitSamplers()
{
	// Initialize the sampler
	sampler = luisa::make_unique<independent_sampler>(this, stream);
	stream << synchronize();
}

} // namespace MechEngine::Rendering
