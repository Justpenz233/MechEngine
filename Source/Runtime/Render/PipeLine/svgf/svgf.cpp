//
// Created by Mayn on 2024/11/16.
//

#include "svgf.h"
namespace MechEngine::Rendering
{

// @see https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl#L93
Float3 svgf::temporal_filter(const ray_intersection& intersection, const Float3& pixel_color) const
{
	const float NORMAL_TOLERANCE = 5.0e-2;
	Float2 pre_pixel_pos = intersection.motion_vector;
	UInt2 pre_pixel_coord = UInt2(pre_pixel_pos);

	auto pixel_pos_frac = pre_pixel_pos - make_float2(pre_pixel_coord);
	auto inv_pixel_pos_frac = 1.f - pixel_pos_frac;
	Float weights[4];
	weights[0] = inv_pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[1] = pixel_pos_frac.x * inv_pixel_pos_frac.y;
	weights[2] = inv_pixel_pos_frac.x * pixel_pos_frac.y;
	weights[3] = pixel_pos_frac.x * pixel_pos_frac.y;

	Float3 sum_val = make_float3(0.f);
	Float sum_weight = 0.f;
	auto WinSize = buffer.color.size();
	for(int dy = 0; dy <= 1; dy += 1)
	{
		for (int dx = 0; dx <= 1; dx += 1)
		{
			auto p = make_uint2(dx, dy) + pre_pixel_coord;
			$if (all(p > make_uint2(0, 0)) & all(p < WinSize))
			{
				auto pre_instance = buffer.instance_id->read(p).x;
				auto pre_normal = buffer.normal->read(p).xyz();
				$if(pre_instance == intersection.instance_id &
					distance_squared(pre_normal, intersection.corner_normal_world) < NORMAL_TOLERANCE)
				{
					auto pre_color = buffer.color->read(p);
					sum_weight += weights[dx + dy * 2];
					sum_val += pre_color.xyz() * weights[dx + dy * 2];
				};
			};
		}
	}
	sum_val /= ite(sum_weight > 0.f, sum_weight, 1.f);
	return ite(sum_weight > 0.f, lerp(pixel_color, sum_val, 0.8f), pixel_color);
}

}