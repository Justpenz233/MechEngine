//
// Created by Mayn on 2024/11/2.
//

#pragma once
#include "GpuScene.h"
#include "ris_reservoir.h"
#include "svgf/svgf.h"

namespace MechEngine::Rendering
{
class ENGINE_API PathTracingScene : public GpuScene
{
public:
	float3 wireframe_color = make_float3(0.f);

	using GpuScene::GpuScene;

	virtual void LoadRenderSettings() override;

	virtual void InitBuffers() override;

	virtual void CompileShader() override;

	virtual void PrePass(CommandList& CmdList) override;

	virtual void PostPass(CommandList& CmdList) override;

	virtual void render_main_view(const UInt& frame_index, const UInt& time) override;

	ray_intersection intersect_bias(const UInt2& pixel_coord, Expr<Ray> ray, Bool first_intersect);
	/**
	 * multi important sampling path tracing
	 * @param ray the ray to calculate
	 * @param pixel_pos the position of the pixel
	 * @param weight the weight of the pixel
	 * @return pixel color
	 */
	Float3 mis_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight = 1.f);

	/**
	 * resampling important sampling path tracing
	 * @param ray the ray to calculate
	 * @param pixel_pos the position of the pixel
	 * @param pixel_coord the coordination of the pixel
	 * @param weight the weight of the pixel
	 * @return
	 */
	// Float3 ris_path_tracing(Var<Ray> ray, const Float2& pixel_pos, const UInt2& pixel_coord, const Float& weight = 1.f);

	/**
	 * Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	 * Currently, we only draw the first intersection with the wireframe, which means
	 * the wireframe would disappear in refractive or alpha blended surface
	 * @see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	 * @see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	*/
	Float wireframe_intensity(const ray_intersection& intersection, const Float2& pixel_pos) const;

	auto get_reservoir(const UInt2& pixel_coord) const
	{
		return reservoirs->read(pixel_coord.x + pixel_coord.y * GetWindosSize().x);
	}

	auto set_reservoir(const UInt2& pixel_coord, Expr<ris_reservoir> reservoir)
	{
		reservoirs->write(pixel_coord.x + pixel_coord.y * GetWindosSize().x, reservoir);
	}

protected:
	bool bUseSVGF = true;
	bool bUseRIS = true;

	BufferView<ris_reservoir> reservoirs;

	unique_ptr<svgf> svgf;
};
};