//
// Created by Mayn on 2025/1/5.
//

#include "scanline_rasterizer.h"
#include "Render/PipeLine/GpuScene.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
namespace MechEngine::Rendering
{
void scanline_rasterizer::CompileShader(Device& Device)
{
	auto WinSize = scene->GetWindosSize();
	triangle_box = Device.create_buffer<float4>(triangle_max_number);

	vbuffer.bary = Device.create_image<float>(PixelStorage::FLOAT2, WinSize.x, WinSize.y);
	vbuffer.instance_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);
	vbuffer.triangle_id = Device.create_image<uint>(PixelStorage::INT1, WinSize.x, WinSize.y);

	RasterMeshShader = luisa::make_unique<decltype(RasterMeshShader)::element_type>(
		Device.compile<1>([&](const UInt& instance_id, const UInt& mesh_id) noexcept {
			raster_mesh(instance_id, mesh_id);
		}));

	RasterTriangleShader = luisa::make_unique<decltype(RasterTriangleShader)::element_type>(
		Device.compile<2>([&](const UInt& instance_id, const UInt& mesh_id, const UInt& triangle_id, const UInt2& pixel_delta) noexcept {
			raster_triangle(instance_id, mesh_id, triangle_id, pixel_delta);
		}));

	ClearShader = luisa::make_unique<decltype(ClearShader)::element_type>(
		Device.compile<2>([&]() noexcept {
			vbuffer.instance_id->write(dispatch_id().xy(), make_uint4(~0u));
		}));
}
void scanline_rasterizer::ClearPass(Stream& stream)
{
	stream << (*ClearShader)().dispatch(scene->GetWindosSize());
}

void scanline_rasterizer::VisibilityPass(Stream& stream, uint instance_id, uint mesh_id, uint triangle_count)
{
	static vector<float4> triangle_boxes(triangle_max_number);

	stream
	<< (*RasterMeshShader)(instance_id, mesh_id).dispatch(triangle_count)
	<< synchronize();

	// TODO: use indirect dispatch optimize this write back
	stream << triangle_box.copy_to(triangle_boxes.data());

	for(uint i = 0; i < triangle_count; i++)
	{
		auto& box = triangle_boxes[i];
		auto min_pixel = make_uint2(uint(box.x - 0.5f), uint(box.y - 0.5f));
		auto max_pixel = make_uint2(uint(box.z + 0.5f), uint(box.w + 0.5f));
		auto size = max_pixel - min_pixel;
		if(size.x == 0 || size.y == 0) continue;
		// LOG_INFO("min_pixel: {}, max_pixel: {}", min_pixel, max_pixel);
		stream << (*RasterTriangleShader)(instance_id, mesh_id, i, min_pixel).dispatch(size);
	}
	stream << synchronize();
}

void scanline_rasterizer::raster_mesh(const UInt& instance_id, const UInt& mesh_id) const
{
	auto triangle_id = dispatch_id().x;

	auto view = get_view();
	auto vertices = get_vertices(mesh_id, triangle_id);
	auto model_transform = get_instance_transform_mat(instance_id);

	ArrayFloat2<3> screen_coords;
	for (int i = 0; i < 3; i++)
	{
		auto vertex = vertices[i];
		auto world_pos = model_transform * Float4(vertex.px, vertex.py, vertex.pz, 1.0f);
		screen_coords[i] = view->world_to_pixel(world_pos.xyz());
	}

	// TODO: add back face culling

	auto minX = min(screen_coords[0].x, min(screen_coords[1].x, screen_coords[2].x));
	auto maxX = max(screen_coords[0].x, max(screen_coords[1].x, screen_coords[2].x));
	auto minY = min(screen_coords[0].y, min(screen_coords[1].y, screen_coords[2].y));
	auto maxY = max(screen_coords[0].y, max(screen_coords[1].y, screen_coords[2].y));
	minX = max(0.0f, minX);
	maxX = min(Float(view->viewport_size.x), maxX);
	minY = max(0.0f, minY);
	maxY = min(Float(view->viewport_size.y), maxY);

	triangle_box->write(triangle_id, make_float4(minX, minY, maxX, maxY));
}

void scanline_rasterizer::raster_triangle(const UInt& instance_id, const UInt& mesh_id, const UInt& triangle_id, const UInt2& pixel_delta) const
{
	auto& g_buffer = scene->get_gbuffer();
	auto pixel = dispatch_id().xy() + pixel_delta;
	auto pixel_coord = make_float2(pixel) + 0.5f;
	auto vertices = get_vertices(mesh_id, triangle_id);
	auto model_transform = get_instance_transform_mat(instance_id);

	auto view = get_view();
	ArrayFloat3<3> screen_coords;
	for (int i = 0; i < 3; i++)
	{
		auto vertex = vertices[i];
		auto world_pos = model_transform * Float4(vertex.px, vertex.py, vertex.pz, 1.0f);
		screen_coords[i] = view->world_to_screen(world_pos.xyz());
	}
	auto bary = barycentric(pixel_coord, screen_coords[0].xy(), screen_coords[1].xy(), screen_coords[2].xy());
	$if (bary.x > 0.0f & bary.y > 0.0f & bary.x + bary.y <= 1.0f)
	{
		// Z test
		auto z = triangle_interpolate(bary, screen_coords[0].z, screen_coords[1].z, screen_coords[2].z);
		auto flat_index = g_buffer.flattend_index(pixel);
		// g_buffer.depth->atomic(flat_index).fetch_min(z);
		// $if (g_buffer.depth->read(flat_index) == z)
		// {
			vbuffer.instance_id->write(pixel, make_uint4(instance_id));
			vbuffer.triangle_id->write(pixel, make_uint4(triangle_id));
			vbuffer.bary->write(pixel, make_float4(bary, 0.0f, 0.0f));
		// };
	};
}

} // namespace MechEngine::Rendering