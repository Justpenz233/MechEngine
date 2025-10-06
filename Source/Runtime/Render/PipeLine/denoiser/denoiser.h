//
// Created by MarvelLi on 2025/10/2.
//

#pragma once
#include "Render/Core/geometry_buffer.h"
#include "Render/PipeLine/RenderPass.h"
#include "luisa/backends/ext/denoiser_ext.h"

#include <luisa/luisa-compute.h>
namespace MechEngine::Rendering
{
class GpuScene;
}
namespace MechEngine::Rendering
{
struct ray_intersection;
using namespace luisa::compute;

/**
 * Temporal Filtering and OIDN denoiser
 */
class denoiser : public RenderPass
{
	static inline Float luminance(const Float3& c) {
		return c.x * 0.2126f + c.y * 0.7152f + c.z * 0.0722f;
	}
public:

	denoiser(GpuScene* InScene);

	~denoiser() = default;
	/**
 * Run the temporal filter, store the history buffer
 * @param pixel_coord the coordination of the pixel
 * @param intersection the intersection of the ray
 * @param pixel_color the noised color of the pixel
 * @param g_buffer the geometry buffer which store the last frame information
 * @return the filtered color
 */
	Float3 temporal_filter(const UInt2& pixel_coord, const ray_intersection& intersection, const Float3& pixel_color, const geometry_buffer& g_buffer) const;

	virtual void InitPass(Device& Device, luisa::compute::CommandList& command_list) override;

	virtual void CompileShader(Device& Device, bool bDebugInfo) override;

	virtual void PostPass(luisa::compute::CommandList& command_list) const override;

protected:

	bool bUseOIDN = false;

	GpuScene* scene;
	Image<uint> history_length;
	uint2 WinSize;

	Bool is_reproject_valid(const UInt2& pixel_coord, const UInt2& pre_coord, const ray_intersection& intersection, const geometry_buffer& g_buffer) const;


	shared_ptr<DenoiserExt::Denoiser> denoiser_ext;
	Buffer<float4> albedo;
	Buffer<float4> normal;
	Buffer<float4> noisy_image;
	Buffer<float4> output_image;
	unique_ptr<Shader2D<>> copy_frame_buffer_shader;
	unique_ptr<Shader2D<>> write_frame_buffer_shader;
};
};