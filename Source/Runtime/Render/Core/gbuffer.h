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


    /***
     * Fill all the color of the pixel at pixel_coord with given color.
     * Used for clear the gbuffer
     */
    void fill_color(const luisa::compute::UInt2& pixel_coord,const luisa::compute::Float4& color) const noexcept
    {
        base_color->write(pixel_coord, color);
        normal->write(pixel_coord, color);
        // depth->write(pixel_coord, color);
        // instance_id->write(pixel_coord, color);
        // material_id->write(pixel_coord, color);
    }
};
};