//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
struct gbuffer
{
    luisa::compute::Image<float> base_color;
    luisa::compute::Image<float> normal;
    luisa::compute::Image<float> depth;
    luisa::compute::Image<uint> instance_id;
    luisa::compute::Image<uint> material_id;
    // The frame buffer to store the final image, ownership should managed by the window.
    luisa::compute::Image<float>* frame_buffer{nullptr};


    void set_default(const luisa::compute::UInt2& pixel_coord,
        const luisa::compute::Float4& background_color = luisa::make_float4(1.)) const noexcept
    {
        base_color->write(pixel_coord, background_color);
        normal->write(pixel_coord, background_color);
        depth->write(pixel_coord, luisa::compute::make_float4(1.));
        instance_id->write(pixel_coord, luisa::compute::make_uint4(~0u));
        material_id->write(pixel_coord, luisa::compute::make_uint4(~0u));
    }
};
};