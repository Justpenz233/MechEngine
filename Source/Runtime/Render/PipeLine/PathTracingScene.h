//
// Created by Mayn on 2024/11/2.
//

#pragma once
#include "GpuScene.h"
namespace MechEngine::Rendering
{
class ENGINE_API PathTracingScene : public GpuScene
{
public:
	float3 wireframe_color = make_float3(0.f);

	using GpuScene::GpuScene;
	virtual void render_main_view(const UInt& frame_index, const UInt& time) override;

	/**
	 * Calculate the color of a pixel with a ray
	 * @param ray the ray to calculate
	 * @param pixel_pos the position of the pixel
	 * @param weight the weight of the pixel
	 * @return pixel color
	 */
	Float3 render_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight = 1.f);

	Float wireframe_intensity(const ray_intersection& intersection, const Float2& pixel_pos) const;

	Float3 reproject_last_frame(const ray_intersection& intersection, const UInt2& pixel_coord, const Float3& pixel_color);
};
};