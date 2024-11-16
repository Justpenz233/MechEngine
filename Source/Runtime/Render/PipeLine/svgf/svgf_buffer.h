//
// Created by Mayn on 2024/11/16.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "Render/Core/ray_tracing_hit.h"
namespace MechEngine::Rendering
{
	using namespace luisa::compute;


	// history buffer used for svgf
	struct svgf_buffer
	{
		Image<uint> instance_id;
		Image<float> color;
		Image<float> depth;
		Image<float> normal;
		svgf_buffer() = default;

		svgf_buffer(Device& device, const uint2& size)
		{
			InitBuffers(device, size);
		}

		void InitBuffers(Device& device, const uint2& size)
		{
			instance_id = device.create_image<uint>(PixelStorage::INT1, size.x, size.y);
			color = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
			depth = device.create_image<float>(PixelStorage::FLOAT1, size.x, size.y);
			normal = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		}

		/**
		 * fill the svgf buffer with the intersection data
		 */
		void fill(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color) const
		{
			instance_id->write(pixel_coord, make_uint4(intersection.instance_id));
			color->write(pixel_coord, make_float4(pixel_color, 1.f));
			depth->write(pixel_coord, make_float4(intersection.depth));
			normal->write(pixel_coord, make_float4(intersection.corner_normal_world, 1.f));
		}
	};
}
