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
	virtual void CompileShader(Device& Device) override;

	virtual void ClearPass(Stream& stream) override;

	virtual void VisibilityPass(Stream& stream, uint instance_id, uint mesh_id, uint triangle_count) override;

protected:
	void raster_mesh(const UInt& instance_id, const UInt& mesh_id) const;

	void raster_triangle(const UInt& instance_id, const UInt& mesh_id, const UInt& triangle_id, const UInt2& pixel_delta) const;

protected:
	static constexpr uint triangle_max_number = (1<<20);

	// lx, rx, ly, ry for triangle bounding box in screen space
	Buffer<float4> triangle_box;

	unique_ptr<Shader2D<>> ClearShader;

	// instance id
	unique_ptr<Shader1D<uint, uint>> RasterMeshShader;

	// instance id, triangle id, pixel coord delta
	unique_ptr<Shader2D<uint, uint, uint, uint2>> RasterTriangleShader;


};
};
