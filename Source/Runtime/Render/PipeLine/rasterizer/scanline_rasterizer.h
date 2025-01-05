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


	// Visibility pass shader
	// instance id, triangle id, pixel coord delta
	unique_ptr<Shader2D<uint, uint, uint2>> VisibilityShader;


};
};
