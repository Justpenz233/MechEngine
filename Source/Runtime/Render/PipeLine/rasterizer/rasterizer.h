//
// Created by Mayn on 2025/1/5.
//

#pragma once
#include "visibility_buffer.h"
#include <Render/PipeLine/RenderPass.h>

namespace MechEngine::Rendering
{

class GpuScene;
struct Vertex;
struct view;

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

	UInt get_mesh_id(const UInt& instance_id) const;

	Float4x4 get_instance_transform_mat(const UInt& instance_id) const;

	[[nodiscard]] ArrayVar<Vertex, 3> get_vertices(const UInt& mesh_id, const UInt& triangle_id) const;

	[[nodiscard]] Var<view> get_view() const;

protected:
	GpuScene* scene;
	visibility_buffer vbuffer;
};

};