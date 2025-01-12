//
// Created by Mayn on 2024/11/16.
//

#include "svgf.h"
#include "Render/Core/math_function.h"
namespace MechEngine::Rendering
{

// @see https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl#L93
Float3 svgf::temporal_filter(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color) const
{
	const float NORMAL_TOLERANCE = 5.0e-2;
	Float2		pre_pixel_pos = intersection.motion_vector;
	UInt2		pre_pixel_coord = UInt2(pre_pixel_pos);

	auto  pixel_pos_frac = pre_pixel_pos - make_float2(pre_pixel_coord);
	auto  inv_pixel_pos_frac = 1.f - pixel_pos_frac;
	Float weights[4];
	weights[0] = inv_pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[1] = pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[2] = inv_pixel_pos_frac.x * pixel_pos_frac.y;
	weights[3] = pixel_pos_frac.x * pixel_pos_frac.y;

	Float3 sum_val = make_float3(0.f);
	Float  sum_spp = 0.f;
	Float  sum_moment = 0.f;
	Float  sum_weight = 0.f;

	auto WinSize = buffer.color.size();
	for (int dy = 0; dy <= 1; dy += 1)
	{
		for (int dx = 0; dx <= 1; dx += 1)
		{
			auto p = make_uint2(dx, dy) + pre_pixel_coord;
			$if(all(p > make_uint2(0, 0)) & all(p < WinSize))
			{
				auto pre_instance = buffer.instance_id->read(p).x;
				auto pre_normal = buffer.normal->read(p).xyz();
				$if(pre_instance == intersection.instance_id & distance_squared(pre_normal, intersection.corner_normal_world) < NORMAL_TOLERANCE)
				{
					auto pre_color = buffer.color->read(p);
					sum_weight += weights[dx + dy * 2];
					sum_val += pre_color.xyz() * weights[dx + dy * 2];
					sum_spp += pre_color.w * weights[dx + dy * 2];
					sum_moment += buffer.moment->read(p).x * weights[dx + dy * 2];
				};
			};
		}
	}
	$if(sum_weight > 0.f)
	{
		sum_val /= sum_weight;
		sum_spp /= sum_weight;
		sum_moment /= sum_weight;
	};
	auto new_color = ite(sum_weight > 0.f, lerp(sum_val, pixel_color, 0.15f), pixel_color);

	auto moment_a = max(1.0f / (sum_spp + 1.0f), 0.2f);
	auto new_moment = lerp(sum_moment, square(luminance(pixel_color)), moment_a);

	buffer.instance_id->write(pixel_coord, make_uint4(intersection.instance_id));
	buffer.color->write(pixel_coord, make_float4(new_color, sum_spp + 1.f));
	buffer.normal->write(pixel_coord, make_float4(intersection.corner_normal_world, intersection.depth));
	buffer.moment->write(pixel_coord, make_float4(new_moment));
	return new_color;
}

Float3 svgf::atrous_filter(const UInt2& pixel_coord, const UInt& step_size) const
{
	const float NORMAL_PHI = 128;
	const float DEPTH_PHI = 1;
	const float LUMINANCE_PHI = 4;
	const Float3 kernel_weights = make_float3( 1.0f, 2.0f / 3.0f, 1.0f / 6.0f );

	auto WinSize = buffer.color.size();


	// First fetch current information
	auto normal = buffer.normal->read(pixel_coord).xyz();
	auto color = buffer.color_1->read(pixel_coord).xyz();
	auto moment = buffer.moment->read(pixel_coord).x;
	auto depth = buffer.normal->read(pixel_coord).w;
	auto l = luminance(color);

	Float3 new_color = make_float3(0.f);
	Float sum_weight = 0.f;
	$for(dx, -2, 3)
	{
		$for(dy, -2, 3)
		{
			auto p = make_float2(pixel_coord) + make_float2(Float(dx), Float(dy)) * Float(step_size);
			auto p_coord = UInt2(p);
			$if (p.x < 0 | p.y < 0 | p.x >= WinSize.x | p.y >= WinSize.y){$continue;};
			$if (buffer.instance_id->read(p_coord).x != buffer.instance_id->read(pixel_coord).x){$continue;};
			auto kernel_weight = kernel_weights[Int(abs(dx))] * kernel_weights[Int(abs(dy))];
			auto p_normal = buffer.normal->read(p_coord).xyz();
			auto p_color = buffer.color_1->read(p_coord).xyz();
			auto p_l = luminance(p_color);
			auto p_moment = buffer.moment->read(p_coord).x;
			auto p_depth = buffer.normal->read(p_coord).w;

			auto dxdz = buffer.normal->read(UInt2(p + sign(Float(dx)) * make_float2(1.f, 0.f))).w - depth;
			auto dydz = buffer.normal->read(UInt2(p + sign(Float(dy)) * make_float2(0.f, 1.f))).w - depth;

			auto w_normal = pow(saturate(dot(normal, p_normal)), NORMAL_PHI);
			auto w_z = exp(-abs(depth - p_depth) / (abs(dxdz*dx + dydz*dy) * DEPTH_PHI + 0.0001f));
			// auto w_l = exp(abs(l - p_l) / (abs(l) + 0.0001f));

			Float kx = kernel_weights[Int(abs(dx))];
			Float ky = kernel_weights[Int(abs(dy))];
			auto w_i = w_normal * w_z * kernel_weight;
			// auto w_i =  kernel_weight;

			w_i *= kx * ky;

			new_color += p_color * w_i;
			sum_weight += w_i;
		};
	};
	return ite(sum_weight > 0.f, new_color / sum_weight, color);
}

void svgf::CompileShader(Device& device, bool bDebugInfo)
{
	buffer = svgf_buffer(device, WinSize);

	spacial_filter_shader = luisa::make_unique<Shader2D<uint>>(device.compile<2>(
		[&](UInt step_size) noexcept {
			auto pixel_coord = dispatch_id().xy();
			buffer.color_1->write(pixel_coord, buffer.color->read(pixel_coord));
			synchronize();

			auto new_color = atrous_filter(pixel_coord, step_size);
			buffer.color->write(pixel_coord, make_float4(new_color, 1.f));
		}, {.enable_debug_info = bDebugInfo, .name = "SpacialFilterShader"}));

	write_frame_buffer_shader = luisa::make_unique<Shader2D<>>(device.compile<2>(
		[&]() noexcept {
			auto pixel_coord = dispatch_id().xy();
			auto color = buffer.color->read(pixel_coord);
			frame_buffer->write(pixel_coord, make_float4(color.xyz(), 1.f));
		}, {.enable_debug_info = bDebugInfo, .name = "SVGFWriteFrameBuffer"}));
}

void svgf::PostPass(Stream& stream) const
{
	CommandList command_list{};
	for(int i = 0;i < 4;i ++)
		command_list << (*spacial_filter_shader)(1 << i).dispatch(frame_buffer.size());
	command_list << (*write_frame_buffer_shader)().dispatch(frame_buffer.size());
	stream << command_list.commit();
}

} // namespace MechEngine::Rendering