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

	virtual void VisibilityPass(Stream& stream) override;

protected:
	void raster_mesh(const UInt& instance_id) const;

	void raster_triangle(const UInt& instance_id, const UInt& triangle_id, const UInt2& pixel_delta) const;

protected:
	// lx, rx, ly, ry for triangle bounding box in screen space
	Buffer<float4> triangle_box;

	// instance id
	unique_ptr<Shader1D<uint>> RasterMesh;

	// instance id, triangle id, pixel coord delta
	unique_ptr<Shader2D<uint, uint, uint2>> RasterTriangle;


};
};
