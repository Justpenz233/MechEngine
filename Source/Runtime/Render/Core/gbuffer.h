//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "Render/material/material_base.h"

namespace MechEngine::Rendering
{
struct gbuffer
{
    Image<float> base_color;
    Image<float> normal;
    Image<float> depth;
    Image<uint> instance_id;
    Image<uint> material_id;
    // The frame buffer to store the final image, ownership should managed by the window.
    Image<float>* frame_buffer{nullptr};


    void set_default(const UInt2& pixel_coord,
        const Float4& background_color = luisa::make_float4(1.)) const noexcept
    {
        base_color->write(pixel_coord, background_color);
        normal->write(pixel_coord, background_color);
        depth->write(pixel_coord, make_float4(1.));
        instance_id->write(pixel_coord, make_uint4(~0u));
        material_id->write(pixel_coord, make_uint4(~0u));
    }

	void write(const UInt2& pixel_coord,
		const material_parameters& material,
		const ray_intersection& intersection) noexcept
    {
    	instance_id->write(pixel_coord, make_uint4(intersection.instace_id));
    	material_id->write(pixel_coord, make_uint4(intersection.material_id));
    	base_color->write(pixel_coord, make_float4(material.base_color, 1.f));
    	normal->write(pixel_coord, make_float4(material.normal, 1.f));
    	depth->write(pixel_coord, make_float4(intersection.depth));
    }
};
};