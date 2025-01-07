//
// Created by Mayn on 2024/11/16.
//

#pragma once
#include "svgf_buffer.h"
#include "Render/Core/geometry_buffer.h"
#include "Render/PipeLine/RenderPass.h"

namespace MechEngine::Rendering
{
/**
 * Spatio-Temporal Variance Guided Filtering
 * @ref https://cg.ivd.kit.edu/publications/2017/svgf/svgf_preprint.pdf
 * @ref https://github.com/TheVaffel/spatiotemporal-variance-guided-filtering/blob/master/svgf.cl
 */
class svgf : public RenderPass
{
public:
	svgf(const uint2& size, const ImageView<float>& in_frame_buffer) :
	frame_buffer(in_frame_buffer), WinSize(size) {}

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

	virtual void PostPass(Stream& stream) const override;

	virtual void CompileShader(Device& device, bool bDebugInfo) override;
protected:
	ImageView<float> frame_buffer;
	uint2 WinSize;

	svgf_buffer buffer;

	unique_ptr<Shader2D<uint>> spacial_filter_shader;
	unique_ptr<Shader2D<>> write_frame_buffer_shader;

	Float3 atrous_filter(const UInt2& pixel_coord, const UInt& step_size) const;
};


};