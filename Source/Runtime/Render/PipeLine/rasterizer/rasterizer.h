//
// Created by Mayn on 2025/1/5.
//

#pragma once
#include "visibility_buffer.h"
#include <Render/PipeLine/RenderPass.h>

namespace MechEngine::Rendering
{
class GpuScene;
}
namespace MechEngine::Rendering
{

/**
 * Base class for visibility-buffer rasterizer
 */
class rasterizer : public RenderPass
{
public:
	explicit rasterizer(GpuScene* InScene) : scene(InScene) {}

	/**
	 * Raster visibility pass
	 */
	virtual void VisibilityPass(Stream& stream) = 0;


protected:
	GpuScene* scene;
	visibility_buffer vbuffer;
};

};