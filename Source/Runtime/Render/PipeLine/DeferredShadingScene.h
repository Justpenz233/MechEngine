//
// Created by Mayn on 2024/11/2.
//

#pragma once
#include "GpuScene.h"
namespace MechEngine::Rendering
{

class ENGINE_API DeferredShadingScene : public GpuScene
{
public:
	using GpuScene::GpuScene;
	virtual void render_main_view(const UInt& frame_index, const UInt& time) override;

	std::pair<Float3, Float> calc_surface_point_color(
		Var<Ray> ray, const ray_intersection& intersection, bool global_illumination);

	Float3 render_pixel(Var<Ray> ray, const Float2& pixel_pos);

};

}
