//
// Created by MarvelLi on 2024/5/1.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "math_function.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    struct view
    {
        uint projection_type = 0; // 0 for perspective, 1 for orthographic

        float aspect_ratio{};
    	float tan_half_fovh{};
    	float tan_half_fovv{};

        uint2 viewport_size;
    	float4x4 transform_matrix;
        float4x4 view_matrix;
        float4x4 inverse_view_matrix;

        float4x4 projection_matrix;
        float4x4 inverse_projection_matrix;

        float4x4 view_projection_matrix;
        float4x4 inverse_view_projection_matrix;
    };
};

LUISA_STRUCT(MechEngine::Rendering::view,
    projection_type, aspect_ratio, tan_half_fovh, tan_half_fovv, viewport_size, transform_matrix, view_matrix, inverse_view_matrix, projection_matrix, inverse_projection_matrix, view_projection_matrix, inverse_view_projection_matrix)
{
    [[nodiscard]] auto generate_ray(const luisa::compute::UInt2& pixel_coord)
    {
    	auto pixel       = make_float2(pixel_coord) + .5f;
    	auto resolution  = make_float2(viewport_size);
    	auto p           = (pixel * 2.0f - resolution) / resolution;
    	p = make_float2(p.x * 1.f, p.y * -1.0f);
    	auto direction      = normalize(make_float3(1.f, p.x * tan_half_fovh, p.y * tan_half_fovv));
		auto origin         = make_float3(transform_matrix[3]);
		auto world_direction = normalize(make_float3x3(transform_matrix) * direction);
		return make_ray(make_float3(origin), world_direction);
    }

    [[nodiscard]] auto world_to_clip(const luisa::compute::Float3& world_position) const noexcept
    {
        auto clip_position = view_projection_matrix * make_float4(world_position, 1.0f);
        return clip_position;
    }

	[[nodiscard]] auto clip_to_ndc(const luisa::compute::Float4& clip_position) const noexcept
    {
	    auto ndc_position = clip_position / clip_position.w;
    	return ndc_position.xyz();
    }

    [[nodiscard]] auto world_to_ndc(const luisa::compute::Float3& world_position) const noexcept
    {
        auto clip_position = view_projection_matrix * make_float4(world_position, 1.0f);
        clip_position /= clip_position.w;
        return clip_position.xyz();
    }

    [[nodiscard]] auto ndc_to_screen(const luisa::compute::Float3& ndc_position) const noexcept
    {
        auto pixel_coord = make_float2(ndc_position.x * 0.5f + 0.5f, -ndc_position.y * 0.5f + 0.5f) * make_float2(viewport_size);
        return pixel_coord;
    }

    [[nodiscard]] auto world_to_screen(const luisa::compute::Float3& world_position) const noexcept
    {
        return ndc_to_screen(world_to_ndc(world_position));
    }

	/**
	* Calculate the intersections between the line and the screen window.
	* With these intersections it knows which portion of the line should be drawn. Matthes¨CDrakopoulos Line Clipping
	* @see https://arxiv.org/abs/1908.01350
	* @see https://www.ncbi.nlm.nih.gov/pmc/articles/PMC9605407/pdf/jimaging-08-00286.pdf
	* @param p0 the screen position start of the line
	* @param p1 the screen position end of the line
	*/
	[[nodiscard]] std::pair<luisa::compute::Float2, luisa::compute::Float2> clamp_to_screen(
		const luisa::compute::Float2& p0, const luisa::compute::Float2& p1)
    {
    	luisa::compute::Float2 x = make_float2(p0.x, p1.x);
    	luisa::compute::Float2 y = make_float2(p0.y, p1.y);
    	$if(!(p0.x < 0 & p1.x < 0) & !(p0.x > viewport_size.x & p1.x > viewport_size.x) &
			!(p0.y < 0 & p1.y < 0) & !(p0.y > viewport_size.y & p1.y > viewport_size.y))
		{
    		luisa::compute::Float xmax = viewport_size.x;
    		luisa::compute::Float ymax = viewport_size.y;
    		$for(i, 0, 2)
    		{
    			$if(x[i] < 0.f)
    			{
    				x[i] = 0.f;
    				y[i] = p0.y + (p1.y - p0.y) * (0.f - p0.x) / (p1.x - p0.x);
    			}
    			$elif(x[i] > xmax)
				{
					x[i] = xmax;
					y[i] = p0.y + (p1.y - p0.y) * (xmax - p0.x) / (p1.x - p0.x);
				};
    			$if(y[i] < 0.f)
				{
					y[i] = 0.f;
					x[i] = p0.x + (p1.x - p0.x) * (0.f - p0.y) / (p1.y - p0.y);
				}
    			$elif(y[i] > ymax)
    			{
    				y[i] = ymax;
    				x[i] = p0.x + (p1.x - p0.x) * (ymax - p0.y) / (p1.y - p0.y);
    			};
    		};
		};
    	return std::make_pair(make_float2(x[0], y[0]), make_float2(x[1], y[1]));
	}

	/**
	* Clip the line to the normalized device coordinates.
	* @see https://www.mdpi.com/1999-4893/16/4/201
	* @param p0 the ndc position start of the line
	* @param p1 the ndc position end of the line
	*/
	std::pair<luisa::compute::Float3, luisa::compute::Float3> clamp_to_ndc(
		const luisa::compute::Float3& p0, const luisa::compute::Float3& p1)
    {
		return MechEngine::Rendering::clip_segment_within_box_3D(
				p0, p1,
				luisa::compute::make_float3(-1.f, -1.f, -1.f),
				luisa::compute::make_float3(1.f, 1.f, 1.f));
    }
};