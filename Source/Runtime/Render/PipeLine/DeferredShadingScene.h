//
// Created by Mayn on 2024/11/2.
//

#pragma once
#include "GpuScene.h"

namespace MechEngine::Rendering
{
class rasterizer;

class ENGINE_API DeferredShadingScene : public GpuScene
{
public:
	DeferredShadingScene(Stream& stream, Device& device, ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

	~DeferredShadingScene() override;

	virtual void LoadRenderSettings() override;

	virtual void PrePass(CommandList& CmdList) override;

	virtual void render_main_view(const UInt& frame_index, const UInt& time) override;

	std::pair<Float3, Float> calc_surface_point_color(
		Var<Ray> ray, const ray_intersection& intersection, bool global_illumination);

	Float3 render_pixel(Var<Ray> ray, const UInt2& pixel_coord);

protected:

	/** Whether to use global illumination */
	bool bGlobalIllumination = false;

	/** Whether to render shadow */
	bool bRenderShadow = false;
};

}
