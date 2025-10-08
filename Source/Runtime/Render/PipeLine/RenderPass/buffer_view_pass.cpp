//
// Created by MarvelLi on 2025/2/19.
//

#include "buffer_view_pass.h"

#include "Render/PipeLine/GpuScene.h"
namespace MechEngine::Rendering
{
buffer_view_pass::buffer_view_pass(GpuScene& InScene) : Scene(InScene) {}


Float3 id_to_color(UInt id)
{
	// Use a simple integer hash to generate pseudo-random RGB values
	UInt hashed = id * 1664525u + 1013904223u; // LCG step
	Float r = ((hashed >> 16) & 0xFF) / 255.0f;
	Float g = ((hashed >> 8) & 0xFF) / 255.0f;
	Float b = (hashed & 0xFF) / 255.0f;
	return {r, g, b};
}

void buffer_view_pass::CompileShader(Device& Device, bool bDebugInfo)
{
	BufferViewShader = make_unique<Shader2D<uint>>(Device.compile<2>([&](UInt ViewMode) {
		auto pixel_coord = dispatch_id().xy();
		$switch(ViewMode)
		{
			$case(static_cast<uint>(ViewMode::DepthBuffer))
			{
				Scene.frame_buffer()->write(pixel_coord, make_float4(Scene.get_gbuffer().read_depth(pixel_coord)));
			};
			$case(static_cast<uint>(ViewMode::NormalWorldBuffer))
			{
				auto normal = Scene.get_gbuffer().normal->read(pixel_coord);
				// Map from [-1, 1] to [0, 1]
				normal = normal * 0.5f + 0.5f;
				Scene.frame_buffer()->write(pixel_coord, normal);
			};
			$case(static_cast<uint>(ViewMode::BaseColorBuffer))
			{
				Scene.frame_buffer()->write(pixel_coord, Scene.get_gbuffer().albedo->read(pixel_coord));
			};
			$case(static_cast<uint>(ViewMode::InstanceIDBuffer))
			{
				auto instance_id = Scene.get_gbuffer().instance_id->read(pixel_coord).as<UInt>();
				auto color = ite(instance_id == ~0u, make_float3(0.f, 0.f, 0.f), id_to_color(instance_id));
				Scene.frame_buffer()->write(pixel_coord, make_float4(color, 1.f));
			};

		};
	}, {.enable_debug_info = bDebugInfo, .name = "BufferViewShader"}));
}

void buffer_view_pass::PostPass(CommandList& command_list) const
{
	RenderPass::PostPass(command_list);
	command_list << (*BufferViewShader)
		(static_cast<uint>(Scene.GetViewMode())).dispatch(Scene.GetWindosSize());
}
void buffer_view_pass::InitPass(luisa::compute::Device& Device, luisa::compute::CommandList& command_list)
{
	RenderPass::InitPass(Device, command_list);
}

};
