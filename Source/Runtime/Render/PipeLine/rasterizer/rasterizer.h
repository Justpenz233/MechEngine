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


	virtual void ClearPass(Stream& stream) = 0;

	/**
	 * Raster visibility pass
	 */
	virtual void VisibilityPass(Stream& stream, uint instance_id, uint mesh_id, uint triangle_count) = 0;

	[[nodiscard]] UInt get_mesh_id(const UInt& instance_id) const;

	[[nodiscard]] Float4x4 get_instance_transform_mat(const UInt& instance_id) const;

	[[nodiscard]] ArrayVar<Vertex, 3> get_vertices(const UInt& mesh_id, const UInt& triangle_id) const;

	[[nodiscard]] Var<view> get_view() const;
	visibility_buffer vbuffer;
protected:
	GpuScene* scene;

};

};