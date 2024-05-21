//
// Created by MarvelLi on 2024/5/1.
//

#pragma once
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    struct view_data
    {
        uint projection_type = 0; // 0 for perspective, 1 for orthographic

        float aspect_ratio{};
        float fov_h{};
        uint2 viewport_size;
        float4x4 view_matrix;
        float4x4 inverse_view_matrix;

        float4x4 projection_matrix;
        float4x4 inverse_projection_matrix;

        float4x4 view_projection_matrix;
        float4x4 inverse_view_projection_matrix;
    };
};

LUISA_STRUCT(MechEngine::Rendering::view_data,
    projection_type, aspect_ratio, fov_h, viewport_size, view_matrix, inverse_view_matrix, projection_matrix, inverse_projection_matrix, view_projection_matrix, inverse_view_projection_matrix)
{
    [[nodiscard]] auto generate_ray(const luisa::compute::UInt2& pixel_coord)
    {

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
};