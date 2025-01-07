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

}

void scanline_rasterizer::VisibilityPass(Stream& stream)
{

}

void scanline_rasterizer::raster_mesh(const UInt& instance_id) const
{
	auto triangle_id = dispatch_id().x;
	auto mesh_id = get_mesh_id(instance_id);

	auto view = get_view();
	auto vertices = get_vertices(mesh_id, triangle_id);
	auto model_transform = get_instance_transform_mat(instance_id);

	ArrayFloat2<3> screen_coords;
	for (int i = 0; i < 3; i++)
	{
		auto vertex = vertices[i];
		auto world_pos = model_transform * Float4(vertex.px, vertex.py, vertex.px, 1.0f);
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

	triangle_box->write(triangle_id, make_float4(minX, maxX, minY, maxY));
}

void scanline_rasterizer::raster_triangle(const UInt& instance_id, const UInt& triangle_id, const UInt2& pixel_delta) const
{
	auto& g_buffer = scene->get_gbuffer();
	auto pixel = dispatch_id().xy() + pixel_delta;
	auto pixel_coord = make_float2(pixel) + 0.5f;
	auto vertices = get_vertices(get_mesh_id(instance_id), triangle_id);
	auto model_transform = get_instance_transform_mat(instance_id);
	auto view = get_view();
	ArrayFloat3<3> screen_coords;
	for (int i = 0; i < 3; i++)
	{
		auto vertex = vertices[i];
		auto world_pos = model_transform * Float4(vertex.px, vertex.py, vertex.px, 1.0f);
		screen_coords[i] = view->world_to_ndc(world_pos.xyz());
	}
	auto bary = barycentric(pixel_coord, screen_coords[0].xy(), screen_coords[1].xy(), screen_coords[2].xy());
	$if (bary.x > 0.0f & bary.y > 0.0f & bary.x + bary.y <= 1.0f)
	{
		// Z test
		auto z = triangle_interpolate(bary, screen_coords[0].z, screen_coords[1].z, screen_coords[2].z);
		auto flat_index = g_buffer.flattend_index(pixel_coord);
		g_buffer.depth->atomic(flat_index).fetch_min(z);
		$if (g_buffer.depth->read(flat_index) == z)
		{
			vbuffer.instance_id->write(pixel, make_uint4(instance_id));
			vbuffer.triangle_id->write(pixel, make_uint4(triangle_id));
			vbuffer.bary->write(pixel, make_float4(bary, 0.0f, 0.0f));
		};
	};
}

} // namespace MechEngine::Rendering