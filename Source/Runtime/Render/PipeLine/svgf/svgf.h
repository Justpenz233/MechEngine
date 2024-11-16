//
// Created by Mayn on 2024/11/16.
//

#pragma once
#include "svgf_buffer.h"
#include "Render/Core/geometry_buffer.h"

namespace MechEngine::Rendering
{
/**
 * Spatio-Temporal Variance Guided Filtering
 * @ref https://cg.ivd.kit.edu/publications/2017/svgf/svgf_preprint.pdf
 * @ref https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl
 */
class svgf
{
public:
	svgf(Device& device, const uint2& size):
	buffer(device, size) {}


	Float3 temporal_filter(const ray_intersection& intersection, const Float3& pixel_color) const;

	void fill_history_buffer(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color) const
	{
		buffer.fill(pixel_coord, intersection, pixel_color);
	}

protected:
	svgf_buffer buffer;
};


};