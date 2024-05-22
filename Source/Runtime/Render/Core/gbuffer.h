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
	Buffer<float> depth;
    Image<uint> instance_id;
    Image<uint> material_id;
    // The frame buffer to store the final image, ownership should managed by the window.
    Image<float>* frame_buffer{nullptr};


    void set_default(const UInt2& pixel_coord,
        const Float4& background_color = luisa::make_float4(1.)) const noexcept
    {
        base_color->write(pixel_coord, background_color);
        normal->write(pixel_coord, background_color);
        depth->write(flattend_index(pixel_coord), 1.f);
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
    	depth->write(flattend_index(pixel_coord), intersection.depth);
    }

	/**
	 * Get the size of the G-Buffers. The size should be equal to the size of the frame buffer.
	 * @return The size of the G-Buffers.
	 */
	[[nodiscard]] UInt2 size() const noexcept
	{
	    return frame_buffer->size();
	}

	/**
	 * Map the pixel coordinate to the index of the flatten buffer. Use for depth buffer.
	 * @param pixel_coord The pixel coordinate.
	 * @return The index of the flatten buffer.
	 */
	[[nodiscard]] UInt flattend_index(const UInt2& pixel_coord) const
	{
	    return pixel_coord.x * frame_buffer->size().y + pixel_coord.y;
    }

	/**
	 * Map the index of the flatten buffer to the pixel coordinate.
	 * @param index The index of the flatten buffer.
	 * @return The pixel coordinate.
	 */
	[[nodiscard]] UInt2 pixel_coord(const UInt& index) const noexcept
	{
	    return make_uint2(index / frame_buffer->size().y, index % frame_buffer->size().y);
	}
};
};