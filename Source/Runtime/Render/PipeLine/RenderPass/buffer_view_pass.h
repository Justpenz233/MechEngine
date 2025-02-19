//
// Created by MarvelLi on 2025/2/19.
//

#pragma once
#include "Render/PipeLine/RenderPass.h"

namespace MechEngine::Rendering
{
class GpuScene;
}
namespace MechEngine::Rendering
{
using namespace luisa::compute;

class buffer_view_pass : public RenderPass
{
public:
	buffer_view_pass(GpuScene& InScene);

	virtual void CompileShader(Device& Device, bool bDebugInfo) override;
	virtual void PostPass(CommandList& command_list) const override;

protected:
	GpuScene& Scene;
	unique_ptr<Shader2D<uint>> BufferViewShader;
};
}