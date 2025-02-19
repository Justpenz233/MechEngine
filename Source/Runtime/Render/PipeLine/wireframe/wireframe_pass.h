//
// Created by MarvelLi on 2025/1/20.
//

#pragma once
#include "Render/PipeLine/RenderPass.h"
#include <luisa/luisa-compute.h>
namespace MechEngine::Rendering
{
class GpuScene;
class wireframe_pass : public RenderPass
{

public:
	wireframe_pass(GpuScene& InScene);

	virtual void CompileShader(luisa::compute::Device& Device, bool bDebugInfo) override;
	virtual void PostPass(luisa::compute::CommandList& command_list) const override;

protected:
	luisa::float3 wireframe_color = luisa::make_float3(0.f);

	GpuScene& Scene;
	/**
	 * Draw wireframe pass, blend  with the pixel color as Anti-aliasing
	 * Currently, we only draw the first intersection with the wireframe, which means
	 * the wireframe would disappear in refractive or alpha blended surface
	 * @see https://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
	 * @see https://www2.imm.dtu.dk/pubdb/edoc/imm4884.pdf
	 */
	luisa::compute::Float wireframe_intensity(luisa::compute::UInt2 pixel_coord) const;

	eastl::unique_ptr<luisa::compute::Shader2D<>> WireFrameShader;

};
}
