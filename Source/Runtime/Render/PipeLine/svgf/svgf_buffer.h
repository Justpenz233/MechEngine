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
		Image<float> color; // color xyz and spp w
		Image<float> color_1; // swap color buffer for filtering

		Image<float> normal; // normal xyz and depth w
		Image<float> moment; // second raw moment of luminance
		svgf_buffer() = default;

		svgf_buffer(Device& device, const uint2& size)
		{
			InitBuffers(device, size);
		}

		void InitBuffers(Device& device, const uint2& size)
		{
			instance_id = device.create_image<uint>(PixelStorage::INT1, size.x, size.y);
			color = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
			normal = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
			moment = device.create_image<float>(PixelStorage::FLOAT1, size.x, size.y);
			color_1 = device.create_image<float>(PixelStorage::FLOAT4, size.x, size.y);
		}
	};
}
