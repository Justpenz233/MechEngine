//
// Created by MarvelLi on 2025/3/3.
//

#pragma once
#include "Render/PipeLine/RenderPass.h"
#include "luisa/backends/ext/denoiser_ext.h"

namespace MechEngine::Rendering
{

using namespace luisa::compute;
class denoiser_ext : public RenderPass
{
public:
	denoiser_ext(class GpuScene* InScene);
	virtual void CompileShader(Device& Device, bool bDebugInfo) override;
	virtual void PostPass(CommandList& command_list) const override;
protected:
	GpuScene* scene;
	shared_ptr<DenoiserExt::Denoiser> denoiser;
	Buffer<float4> albedo;
	Buffer<float4> normal;
	Buffer<float4> noisy_image;
	Buffer<float4> output_image;

	unique_ptr<Shader2D<>> copy_frame_buffer_shader;
	unique_ptr<Shader2D<>> write_frame_buffer_shader;

};
};
