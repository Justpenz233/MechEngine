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

	static inline Float luminance(const Float3& c) {
		return c.x * 0.2126f + c.y * 0.7152f + c.z * 0.0722f;
	}

	/**
	 * Run the temporal filter, store the history buffer
	 * @param pixel_coord the coordination of the pixel
	 * @param intersection the intersection of the ray
	 * @param pixel_color the noised color of the pixel
	 * @return the filtered color
	 */
	Float3 temporal_filter(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color) const;
protected:
	svgf_buffer buffer;
};


};