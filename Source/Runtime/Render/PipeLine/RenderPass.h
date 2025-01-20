//
// Created by Mayn on 2025/1/4.
//

#pragma once
#include <luisa/luisa-compute.h>

/**
 * Interface for a render pass
 */
class RenderPass
{
public:
	virtual ~RenderPass() = default;
	/**
	 * Load render settings for this pass
	 */
	virtual void LoadRenderSettings() {}
	/**
	 * Compile the shader for this pass and initialize the buffers.
	 */
	virtual void CompileShader(luisa::compute::Device& Device, bool bDebugInfo) = 0;

	virtual void PrePass(luisa::compute::CommandList& command_list) const {}

	virtual void MainPass(luisa::compute::CommandList& command_list) const {}

	virtual void PostPass(luisa::compute::CommandList& command_list) const {}
};
