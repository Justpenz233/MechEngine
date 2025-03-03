//
// Created by MarvelLi on 2025/2/17.
//

#pragma once

#include "Render/PipeLine/RenderPass.h"
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
class GpuScene;
using namespace luisa::compute;
class ground_pass : public RenderPass
{

public:
	ground_pass(GpuScene* InScene, const uint2& size, const ImageView<float>& in_frame_buffer);
	virtual void CompileShader(Device& Device, bool bDebugInfo) override;
	virtual void PostPass(CommandList& command_list) const override;
protected:
	Float3 grid(const UInt2& pixel_coord) const;

protected:

	float3 grid_color = {0.5f, 0.5f, 0.5f};
	float2 ground_size = {100.f, 100.f};
	float2 grid_size = {1.f, 1.f};
	float big_grid_size = 5.f;
	unique_ptr<Shader2D<>> ground_shader;

	GpuScene* scene;
	ImageView<float> frame_buffer;
	uint2 WinSize;
};

} // namespace MechEngine::Rendering
