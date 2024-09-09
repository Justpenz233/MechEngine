//
// Created by Mayn on 2024/9/8.
//

#pragma once

#include "Misc/Platform.h"
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{

    using namespace luisa::compute;

    struct triangle_data
    {
        UInt vertex_id[3];
        Float3 world_pos[3];
        Float3 ndc_pos[3];

        Float3 corner_normal[3];
        Float3 vertex_normal[3];
        Float3 triangle_normal;

        Float3 uv[3];

        [[nodiscard]] FORCEINLINE auto interpolate_world_pos(const Float2& barycentric) const noexcept
		{
			return triangle_interpolate(barycentric, world_pos[0], world_pos[1], world_pos[2]);
		}

    	[[nodiscard]] FORCEINLINE auto interpolate_ndc_pos(const Float2& barycentric) const noexcept
        {
        	return triangle_interpolate(barycentric, ndc_pos[0], ndc_pos[1], ndc_pos[2]);
        }

    	[[nodiscard]] FORCEINLINE auto interpolate_corner_normal(const Float2& barycentric) const noexcept
		{
			return normalize(triangle_interpolate(barycentric, corner_normal[0], corner_normal[1], corner_normal[2]));
		}

		[[nodiscard]] FORCEINLINE auto interpolate_vertex_normal(const Float2& barycentric) const noexcept
		{
			return normalize(triangle_interpolate(barycentric, vertex_normal[0], vertex_normal[1], vertex_normal[2]));
		}

		[[nodiscard]] FORCEINLINE auto interpolate_uv(const Float2& barycentric) const noexcept
		{
			return triangle_interpolate(barycentric, uv[0], uv[1], uv[2]);
		}

    };
};