//
// Created by Mayn on 2025/1/5.
//

#pragma once

#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
using namespace luisa::compute;

struct visibility_buffer
{
	// Buffer of geometry instance id
	Image<uint> instance_id;

	// Buffer of geometry triangle id
	Image<uint> triangle_id;

	// Buffer of geometry barycentric coordinates
	Image<float> bary;
};


};