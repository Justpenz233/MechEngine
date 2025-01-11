//
// Created by Mayn on 2025/1/5.
//

#pragma once
#include "rasterizer.h"

namespace MechEngine::Rendering
{
class scanline_rasterizer : public rasterizer
{
	using rasterizer::rasterizer;

public:
	virtual void CompileShader(Device& Device, bool bDebugInfo) override;

	virtual void ClearPass(Stream& stream) override;

	virtual void VisibilityPass(Stream& stream, uint instance_id, uint mesh_id, uint vertex_num, uint triangle_num) override;

protected:
	/**
	 * Vertex shader
	 * @param instance_id instance id
	 * @param mesh_id mesh id
	 */
	void vertex_shader(const UInt& instance_id, const UInt& mesh_id) const;

	/**
	 * Raster a mesh
	 * @param instance_id instance id
	 * @param mesh_id mesh id
	 */
	void raster_mesh(const UInt& instance_id, const UInt& mesh_id) const;

	/**
	 * Raster a triangle
	 * @param instance_id instance id
	 * @param mesh_id mesh id
	 */
	void raster_triangle(const UInt& instance_id, const UInt& mesh_id) const;

protected:
	static constexpr uint triangle_max_number = (1<<20); // Should be dynamic in the future
	static constexpr uint vertex_max_number = (1<<20); // Should be dynamic

	IndirectDispatchBuffer draw_triangle_dispatch_buffer;

	// lx, rx, ly, ry for triangle bounding box in screen space
	Buffer<uint2> triangle_pixel_offset;
	Buffer<float3> vertex_screen_coords;

	// Indirect dispatch buffer, set dispatch count to mesh triangle number
	unique_ptr<Shader1D<IndirectDispatchBuffer, uint>> ResetDispatchBufferShader;

	// Clear visibility buffer and depth buffer
	unique_ptr<Shader2D<>> ClearScreenShader;

	// Vertex shader
	unique_ptr<Shader1D<uint, uint>> VertexShader;

	// Raster mesh and dispatch draw triangle kernel
	unique_ptr<Shader1D<uint, uint>> RasterMeshShader;

	// Raster triangle to pixel shader
	unique_ptr<Shader2D<uint, uint>> RasterTriangleShader;

};
};
