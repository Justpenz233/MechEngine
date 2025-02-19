//
// Created by MarvelLi on 2025/2/19.
//

#include "buffer_view_pass.h"

#include "Render/PipeLine/GpuScene.h"
namespace MechEngine::Rendering
{
buffer_view_pass::buffer_view_pass(GpuScene& InScene) : Scene(InScene) {}

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
				Scene.frame_buffer()->write(pixel_coord, Scene.get_gbuffer().normal->read(pixel_coord));
			};
			// $case(static_cast<uint>(ViewMode::BaseColorBuffer))
			// {
				// Scene.frame_buffer()->write(pixel_coord, Scene.get_gbuffer().base_color->read(pixel_coord));
			// };
		};
	}, {.enable_debug_info = bDebugInfo, .name = "BufferViewShader"}));
}

void buffer_view_pass::PostPass(CommandList& command_list) const
{
	RenderPass::PostPass(command_list);
	command_list << (*BufferViewShader)
		(static_cast<uint>(Scene.GetViewMode())).dispatch(Scene.GetWindosSize());
}

};
