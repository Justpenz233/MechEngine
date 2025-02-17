//
// Created by MarvelLi on 2025/2/17.
//

#include "ground_pass.h"

#include "Render/Core/view.h"
#include "Render/PipeLine/GpuScene.h"
#include "Render/SceneProxy/CameraSceneProxy.h"

namespace MechEngine::Rendering
{

ground_pass::ground_pass(GpuScene* InScene, const uint2& size, const ImageView<float>& in_frame_buffer)
	: RenderPass(), scene(InScene), frame_buffer(in_frame_buffer), WinSize(size) {}

void ground_pass::CompileShader(Device& Device, bool bDebugInfo)
{
	ground_shader = luisa::make_unique<decltype(ground_shader)::element_type>(
		Device.compile<2>([&]() noexcept {
			auto pixel_coord = dispatch_id().xy();
			auto color = grid(pixel_coord);
			frame_buffer->write(pixel_coord, make_float4(color, 1.f));
		},
			{ .enable_debug_info = bDebugInfo, .name = "GroundShader" }));
}
void ground_pass::PostPass(CommandList& command_list) const
{
	command_list << (*ground_shader)().dispatch(WinSize);
}

Float3 ground_pass::grid(const UInt2& pixel_coord) const
{
	auto view = scene->GetCameraProxy()->get_main_view();
	auto pixel_pos = make_float2(pixel_coord) + 0.5f;
	auto ray = view->generate_ray( pixel_pos);
	Float t = -ray.compressed_origin[2] / ray.compressed_direction[2];
	Float pre_depth = scene->get_gbuffer().read_depth(pixel_coord);
	Float intensity = 0.f;
	Float3 grid_color_ = grid_color;
	$if(t > 0.f & t < pre_depth)
	{
		auto x = ray.compressed_origin[0] + t * ray.compressed_direction[0];
		auto y = ray.compressed_origin[1] + t * ray.compressed_direction[1];

		// Nearest grid x and y
		auto grid_l_x = grid_size.x * trunc(x / grid_size.x);
		auto grid_l_y = grid_size.y * trunc(y / grid_size.y);
		auto grid_r_x = grid_l_x + grid_size.x * sign(x);
		auto grid_r_y = grid_l_y + grid_size.y * sign(y);
		auto ll = view->world_to_pixel(make_float3(grid_l_x, grid_l_y, 0.f));
		auto lr = view->world_to_pixel(make_float3(grid_l_x, grid_r_y, 0.f));
		auto rl = view->world_to_pixel(make_float3(grid_r_x, grid_l_y, 0.f));
		auto rr = view->world_to_pixel(make_float3(grid_r_x, grid_r_y, 0.f));

		auto d1 = min(distance_to_segment(pixel_pos, ll, lr), distance_to_segment(pixel_pos, rl, rr));
		auto d2 = min(distance_to_segment(pixel_pos, ll, rl), distance_to_segment(pixel_pos, lr, rr));
		intensity = exp2(-2.f * square(min(d1, d2))); // I = exp2(-2 * d^2)

		auto fade_factor = 1.0f - clamp(t / 100.f, 0.0f, 1.0f);
		grid_color_ = grid_color * fade_factor;
	};
	auto frame_color = frame_buffer->read(pixel_coord);
	return lerp(frame_color.xyz(), grid_color_, intensity);
}

} // namespace MechEngine::Rendering