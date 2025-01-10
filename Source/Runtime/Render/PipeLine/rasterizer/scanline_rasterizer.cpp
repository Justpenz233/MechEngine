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
	triangle_box = Device.create_buffer<float4>(triangle_max_number);

	vbuffer.bary = Device.create_image<float>(PixelStorage::FLOAT2, WinSize.x, WinSize.y);
	vbuffer.instance_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);
	vbuffer.triangle_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);

	VertexShader = luisa::make_unique<decltype(VertexShader)::element_type>(
		Device.compile<1>([&](const UInt& instance_id, const UInt& mesh_id) noexcept {
			vertex_shader(instance_id, mesh_id);
		}, {.enable_debug_info = bDebugInfo}));

	TriangleBBoxShader = luisa::make_unique<decltype(TriangleBBoxShader)::element_type>(
		Device.compile<1>([&](const UInt& instance_id, const UInt& mesh_id) noexcept {
			raster_mesh(instance_id, mesh_id);
		}, {.enable_debug_info = bDebugInfo}));

	RasterTriangleShader = luisa::make_unique<decltype(RasterTriangleShader)::element_type>(
		Device.compile<2>([&](const UInt& instance_id, const UInt& mesh_id, const UInt& triangle_id, const UInt2& pixel_delta) noexcept {
			raster_triangle(instance_id, mesh_id, triangle_id, pixel_delta);
		}, {.enable_debug_info = bDebugInfo}));

	ClearShader = luisa::make_unique<decltype(ClearShader)::element_type>(
		Device.compile<2>([&]() noexcept {
			$comment("Clear visibility buffer and depth buffer");
			auto& g_buffer = scene->get_gbuffer();
			g_buffer.depth->write(g_buffer.flattend_index(dispatch_id().xy()), 1.f);
			vbuffer.instance_id->write(dispatch_id().xy(), make_uint4(~0u));
		}, {.enable_debug_info = bDebugInfo}));
}
void scanline_rasterizer::ClearPass(Stream& stream)
{
	stream << (*ClearShader)().dispatch(scene->GetWindosSize());
}

void scanline_rasterizer::VisibilityPass(Stream& stream, uint instance_id, uint mesh_id, uint vertex_num, uint triangle_num)
{
	static vector<float4> triangle_boxes(triangle_max_number);
	stream
		<< (*VertexShader)(instance_id, mesh_id).dispatch(vertex_num)
		<< (*TriangleBBoxShader)(instance_id, mesh_id).dispatch(triangle_num)
		<< triangle_box.copy_to(triangle_boxes.data()) << synchronize();

	// TODO: use indirect dispatch optimize this write back
	for (uint i = 0; i < triangle_num; i++)
	{
		auto box = triangle_boxes[i];
		auto  min_pixel = make_int2(int(std::floor(box.x)), int(std::floor(box.y)));
		auto  max_pixel = make_int2(int(std::ceil(box.z)), int(std::ceil(box.w)));
		auto  size = (max_pixel - min_pixel) + make_int2(1, 1);
		if (size.x <= 0 || size.y <= 0)
			continue;
		stream << (*RasterTriangleShader)(instance_id, mesh_id, i, make_uint2(min_pixel)).dispatch(make_uint2(size));
	}
	stream << synchronize();
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

void scanline_rasterizer::raster_mesh(const UInt& instance_id, const UInt& mesh_id) const
{
	auto WinSize = scene->GetWindosSize();
	auto triangle_id = dispatch_id().x;

	$comment("Read vertex screen position");
	auto Indices = get_triangle(mesh_id, triangle_id);
	ArrayFloat3<3> screen_coords;
	screen_coords[0] = vertex_screen_coords->read(Indices.i0);
	screen_coords[1] = vertex_screen_coords->read(Indices.i1);
	screen_coords[2] = vertex_screen_coords->read(Indices.i2);

	// TODO: add back face culling
	$comment("Calulate triangle bounding box");
	auto minX = min(screen_coords[0].x, min(screen_coords[1].x, screen_coords[2].x));
	auto maxX = max(screen_coords[0].x, max(screen_coords[1].x, screen_coords[2].x));
	auto minY = min(screen_coords[0].y, min(screen_coords[1].y, screen_coords[2].y));
	auto maxY = max(screen_coords[0].y, max(screen_coords[1].y, screen_coords[2].y));
	minX = clamp(minX, 0.0f, Float(WinSize.x - 1.f));
	maxX = clamp(maxX, 0.0f, Float(WinSize.x - 1.f));
	minY = clamp(minY, 0.0f, Float(WinSize.y - 1.f));
	maxY = clamp(maxY, 0.0f, Float(WinSize.y - 1.f));

	$comment("write back triangle bounding box");
	triangle_box->write(triangle_id, make_float4(minX, minY, maxX, maxY));
}

void scanline_rasterizer::raster_triangle(const UInt& instance_id, const UInt& mesh_id, const UInt& triangle_id, const UInt2& pixel_delta) const
{
	$comment_with_location("raster triangle");
	auto& g_buffer = scene->get_gbuffer();
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
		g_buffer.depth->atomic(flat_index).fetch_min(z);
		$if (g_buffer.depth->read(flat_index) == z)
		{
			$comment("write back visibility buffer");
			vbuffer.instance_id->write(pixel, make_uint4(instance_id));
			vbuffer.triangle_id->write(pixel, make_uint4(triangle_id));
			vbuffer.bary->write(pixel, make_float4(bary, 0.0f, 0.0f));
		};
	};
}

} // namespace MechEngine::Rendering