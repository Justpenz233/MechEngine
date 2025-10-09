//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "Render/material/shader_base.h"

namespace MechEngine::Rendering
{
struct geometry_buffer
{
	Image<uint> instance_id;
	Image<uint> primitive_id;

	Image<float> radiance; // color of the rendering, not include UI
    Image<float> albedo;
    Image<float> normal;
	Buffer<float> depth; // For atomic operation
	Image<float> motion_vector;

	void InitBuffer(Device& device, const uint2& size)
	{
		LOG_DEBUG("GBuffer initial size: {} {}", size.x, size.y);
		radiance = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		albedo = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		normal = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		depth = device.create_buffer<float>(size.x * size.y);
		instance_id = device.create_image<uint>(PixelStorage::INT1, size.x, size.y);
		primitive_id = device.create_image<uint>(PixelStorage::INT1, size.x, size.y);
		motion_vector = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		LOG_INFO("Init render frame buffer: {} {}", size.x, size.y);
	}

    void set_default(const UInt2& pixel_coord,
        const Float4& background_color = luisa::make_float4(1.)) const noexcept
    {
		radiance->write(pixel_coord, background_color);
        albedo->write(pixel_coord, background_color);
        normal->write(pixel_coord, background_color);
        depth->write(flattend_index(pixel_coord), 1e6f);
        instance_id->write(pixel_coord, make_uint4(~0u));
    	primitive_id->write(pixel_coord, make_uint4(~0u));
    	motion_vector->write(pixel_coord, make_float4(0.f));
    }

	void write(const UInt2& pixel_coord,
		Float3 pixel_radiance,
		const ray_intersection& intersection) const noexcept
    {
		radiance->write(pixel_coord, make_float4(pixel_radiance, 1.f));
		albedo->write(pixel_coord, make_float4(intersection.albedo, 1.f));
    	instance_id->write(pixel_coord, make_uint4(intersection.instance_id));
    	primitive_id->write(pixel_coord, make_uint4(intersection.primitive_id));
    	normal->write(pixel_coord, make_float4(intersection.corner_normal_world, 1.f));
    	depth->write(flattend_index(pixel_coord), intersection.depth);
    	motion_vector->write(pixel_coord, make_float4(intersection.motion_vector, 0.f, 0.f));
    }

	/**
	 * Get the size of the G-Buffers. The size should be equal to the size of the frame buffer.
	 * @return The size of the G-Buffers.
	 */
	[[nodiscard]] UInt2 get_size() const noexcept
	{
	    return instance_id.size();
	}

	/**
	 * Map the pixel coordinate to the index of the flatten buffer. Use for depth buffer.
	 * @param pixel_coord The pixel coordinate.
	 * @return The index of the flatten buffer.
	 */
	[[nodiscard]] UInt flattend_index(const UInt2& pixel_coord) const
	{
	    return pixel_coord.x * get_size().y + pixel_coord.y;
    }

	[[nodiscard]] auto read_depth(const UInt2& pixel_coord) const
    {
	    return depth->read(flattend_index(pixel_coord));
    }

	auto write_depth(const UInt2& pixel_coord, const Float& value) const
	{
	    depth->write(flattend_index(pixel_coord), value);
	}

	/**
	 * Map the index of the flatten buffer to the pixel coordinate.
	 * @param index The index of the flatten buffer.
	 * @return The pixel coordinate.
	 */
	[[nodiscard]] UInt2 pixel_coord(const UInt& index) const noexcept
	{
    	auto size_y = get_size().y;
	    return make_uint2(index / size_y, index % size_y);
	}
};
};