//
// Created by Mayn on 2025/1/5.
//

#include "scanline_rasterizer.h"
#include "Render/PipeLine/GpuScene.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
namespace MechEngine::Rendering
{
void scanline_rasterizer::CompileShader(Device& Device, bool bDebugInfo)
{
	auto WinSize = scene->GetWindosSize();

	vertex_screen_coords = Device.create_buffer<float3>(vertex_max_number);
	triangle_pixel_offset = Device.create_buffer<uint2>(triangle_max_number);

	// The maximum size of the dispatch buffer is 16384
	draw_triangle_dispatch_buffer = Device.create_indirect_dispatch_buffer(16384);

	vbuffer.bary = Device.create_image<float>(PixelStorage::FLOAT2, WinSize.x, WinSize.y);
	vbuffer.instance_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);
	vbuffer.triangle_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);

	VertexShader = luisa::make_unique<decltype(VertexShader)::element_type>(
		Device.compile<1>([&](const UInt& instance_id, const UInt& mesh_id) noexcept {
			vertex_shader(instance_id, mesh_id);
		}, {.enable_debug_info = bDebugInfo, .name = "VertexShader"}));

	CullingTriangleShader = luisa::make_unique<decltype(CullingTriangleShader)::element_type>(
		Device.compile<1>([&](const UInt& instance_id, const UInt& mesh_id, const Bool& enable_back_face_culling) noexcept {
			culling_triangle(instance_id, mesh_id, enable_back_face_culling);
		}, {.enable_debug_info = bDebugInfo, .name = "RasterMeshShader"}));

	RasterTriangleShader = luisa::make_unique<decltype(RasterTriangleShader)::element_type>(
		Device.compile<2>([&](const UInt& instance_id, const UInt& mesh_id) noexcept {
			set_block_size(16, 16, 1);
			raster_triangle(instance_id, mesh_id);
		}, {.enable_debug_info = bDebugInfo, .name = "RasterTriangleShader"}));

	ClearScreenShader = luisa::make_unique<decltype(ClearScreenShader)::element_type>(
		Device.compile<2>([&]() noexcept {
			$comment("Clear visibility buffer and depth buffer");
			auto& g_buffer = scene->get_gbuffer();
			g_buffer.depth->write(g_buffer.flattend_index(dispatch_id().xy()), 1.f);
			vbuffer.instance_id->write(dispatch_id().xy(), make_uint4(~0u));
		}, {.enable_debug_info = bDebugInfo, .name = "RasterClearScreenShader"}));

	ResetDispatchBufferShader = luisa::make_unique<decltype(ResetDispatchBufferShader)::element_type>(
		Device.compile<1>([&](const Var<IndirectDispatchBuffer>& buffer, const UInt& dispatch_count) noexcept {
			buffer.set_dispatch_count(dispatch_count);
		}, {.enable_debug_info = bDebugInfo, .name = "ScanlineResetDispatchBuffer"}));
}

void scanline_rasterizer::ClearPass(CommandList& command_list)
{
	command_list << (*ClearScreenShader)().dispatch(scene->GetWindosSize());
}

void scanline_rasterizer::VisibilityPass(CommandList& command_list, uint instance_id, uint mesh_id, uint vertex_num, uint triangle_num, bool back_face_culling)
{
	static vector<float4> triangle_boxes(triangle_max_number);

	ASSERTMSG(triangle_num <= 16384, "The maximum size of the dispatch buffer is 16384 due to metal limitation");
	command_list
		<< (*VertexShader)(instance_id, mesh_id).dispatch(vertex_num)
		<< (*ResetDispatchBufferShader)(draw_triangle_dispatch_buffer, triangle_num).dispatch(1) // set dispatch count
		<< (*CullingTriangleShader)(instance_id, mesh_id, back_face_culling).dispatch(triangle_num);
	for(int i = 0; i < triangle_num; i ++)
		command_list << (*RasterTriangleShader)(instance_id, mesh_id).dispatch(draw_triangle_dispatch_buffer, i, 1);
}

void scanline_rasterizer::vertex_shader(const UInt& instance_id, const UInt& mesh_id) const
{
	auto vertex_id = dispatch_id().x;
	auto view = get_view();
	$comment("Vertex shader");
	auto model_transform = get_instance_transform_mat(instance_id);
	auto vertex = get_vertex(mesh_id, vertex_id);
	auto world_pos = model_transform * Float4(vertex.px, vertex.py, vertex.pz, 1.0f);
	vertex_screen_coords->write(vertex_id,
		view->world_to_screen(world_pos.xyz()));
}

void scanline_rasterizer::culling_triangle(const UInt& instance_id, const UInt& mesh_id, Bool enable_back_face_culling) const
{
	auto WinSize = scene->GetWindosSize();
	auto triangle_id = dispatch_id().x;

	$comment("Read vertex screen position");
	auto Indices = get_triangle(mesh_id, triangle_id);
	ArrayFloat3<3> screen_coords;
	screen_coords[0] = vertex_screen_coords->read(Indices.i0);
	screen_coords[1] = vertex_screen_coords->read(Indices.i1);
	screen_coords[2] = vertex_screen_coords->read(Indices.i2);

	$comment("Calulate triangle bounding box");
	auto minX = min(screen_coords[0].x, min(screen_coords[1].x, screen_coords[2].x));
	auto maxX = max(screen_coords[0].x, max(screen_coords[1].x, screen_coords[2].x));
	auto minY = min(screen_coords[0].y, min(screen_coords[1].y, screen_coords[2].y));
	auto maxY = max(screen_coords[0].y, max(screen_coords[1].y, screen_coords[2].y));
	auto bfc_result = back_face_culling(screen_coords) & enable_back_face_culling;

	$if(bfc_result | minX > Float(WinSize.x - 1.f) | minY > Float(WinSize.y - 1.f) | maxX < 0.0f | maxY < 0.0f)
	{
		$comment("Triangle is out of screen");
		draw_triangle_dispatch_buffer->set_kernel(
			triangle_id,
			make_uint3(16, 16, 1),
			make_uint3(0u, 1u, 1u),
			triangle_id);
	}
	$else
	{
		$comment("Triangle is inside of screen");
		minX = clamp(minX, 0.0f, Float(WinSize.x - 1.f));
		minY = clamp(minY, 0.0f, Float(WinSize.y - 1.f));
		maxX = clamp(maxX, 0.0f, Float(WinSize.x - 1.f));
		maxY = clamp(maxY, 0.0f, Float(WinSize.y - 1.f));

		minX = floor(minX); maxX = ceil(maxX);
		minY = floor(minY); maxY = ceil(maxY);

		triangle_pixel_offset->write(triangle_id, make_uint2(UInt(minX), UInt(minY)));

		$comment("Set draw triangle dispatch buffer");
		draw_triangle_dispatch_buffer->set_kernel(
			triangle_id,
			make_uint3(16, 16, 1),
			make_uint3(UInt(Int(maxX - minX) + 1), UInt(Int(maxY - minY) + 1), 1u),
			triangle_id);
	};
}

void scanline_rasterizer::raster_triangle(const UInt& instance_id, const UInt& mesh_id) const
{
	$comment_with_location("raster triangle");
	auto& g_buffer = scene->get_gbuffer();
	auto triangle_id = kernel_id();
	auto pixel_delta = triangle_pixel_offset->read(triangle_id).xy();
	auto pixel = dispatch_id().xy() + pixel_delta;
	auto pixel_coord = make_float2(pixel) + 0.5f;

	$comment("Read vertex screen position");
	// TODO move screen_coords as input
	ArrayFloat3<3> screen_coords;
	auto Indices = get_triangle(mesh_id, triangle_id);
	screen_coords[0] = vertex_screen_coords->read(Indices.i0);
	screen_coords[1] = vertex_screen_coords->read(Indices.i1);
	screen_coords[2] = vertex_screen_coords->read(Indices.i2);
	$comment_with_location("interpolate barycentric coordinate");

	auto bary = barycentric(pixel_coord, screen_coords[0].xy(), screen_coords[1].xy(), screen_coords[2].xy());
	$if (bary.x >= 0.0f & bary.y >= 0.0f & bary.x + bary.y <= 1.0f)
	{
		// Z test
		auto z = triangle_interpolate(bary, screen_coords[0].z, screen_coords[1].z, screen_coords[2].z);
		auto flat_index = g_buffer.flattend_index(pixel);

		$comment("depth test");
		// auto pre_z = g_buffer.depth->atomic(flat_index).fetch_min(z);
		auto pre_z = g_buffer.depth->read(flat_index);
		$if (pre_z > z)
		{
			$comment("write back visibility buffer");
			g_buffer.depth->write(flat_index, z);
			vbuffer.instance_id->write(pixel, make_uint4(instance_id));
			vbuffer.triangle_id->write(pixel, make_uint4(triangle_id));
			vbuffer.bary->write(pixel, make_float4(bary, 0.0f, 0.0f));
		};
	};
}

} // namespace MechEngine::Rendering
