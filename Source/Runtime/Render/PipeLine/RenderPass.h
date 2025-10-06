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

	/**
	 * This will be called once when the scene is initialized
	 * Create resources and upload initial data to GPU
	 */
	virtual void InitPass(luisa::compute::Device& Device, luisa::compute::CommandList& command_list) {}

	/**
	 * This will be called before the main render pass
	 * Do some pre-process here
	 */
	virtual void PrePass(luisa::compute::CommandList& command_list) const {}

	/**
	 * This will be called after the main render pass
	 * Do some post-process here
	 */
	virtual void PostPass(luisa::compute::CommandList& command_list) const {}
};
