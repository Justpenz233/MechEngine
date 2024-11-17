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
	Float2 pre_pixel_pos = intersection.motion_vector;
	UInt2 pre_pixel_coord = UInt2(pre_pixel_pos);

	auto  pixel_pos_frac = pre_pixel_pos - make_float2(pre_pixel_coord);
	auto  inv_pixel_pos_frac = 1.f - pixel_pos_frac;
	Float weights[4];
	weights[0] = inv_pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[1] = pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[2] = inv_pixel_pos_frac.x * pixel_pos_frac.y;
	weights[3] = pixel_pos_frac.x * pixel_pos_frac.y;

	Float3 sum_val = make_float3(0.f);
	Float sum_spp = 0.f;
	Float sum_moment = 0.f;
	Float sum_weight = 0.f;

	auto  WinSize = buffer.color.size();
	for (int dy = 0; dy <= 1; dy += 1)
	{
		for (int dx = 0; dx <= 1; dx += 1)
		{
			auto p = make_uint2(dx, dy) + pre_pixel_coord;
			$if(all(p > make_uint2(0, 0)) & all(p < WinSize))
			{
				auto pre_instance = buffer.instance_id->read(p).x;
				auto pre_normal = buffer.normal->read(p).xyz();
				$if(pre_instance == intersection.instance_id &
					distance_squared(pre_normal, intersection.corner_normal_world) < NORMAL_TOLERANCE)
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
	// auto blend_a = max(1.0f / (sum_spp + 1.0f), 0.15f);
	auto new_color = ite(sum_weight > 0.f, lerp(sum_val, pixel_color, 0.15f), pixel_color);

	auto moment_a = max(1.0f / (sum_spp + 1.0f), 0.2f);
	auto new_moment = lerp(sum_moment, square(luminance(pixel_color)), moment_a);

	buffer.instance_id->write(pixel_coord, make_uint4(intersection.instance_id));
	buffer.color->write(pixel_coord, make_float4(new_color, sum_spp + 1.f));
	buffer.normal->write(pixel_coord, make_float4(intersection.corner_normal_world, intersection.depth));
	buffer.moment->write(pixel_coord, make_float4(new_moment));
	return new_color;
}

} // namespace MechEngine::Rendering