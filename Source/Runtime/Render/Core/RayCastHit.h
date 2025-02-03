//
// Created by MarvelLi on 2024/3/28.
//


#pragma once
#include <luisa/luisa-compute.h>


struct RayCastHit
{
	// Instance id of the hit
	uint instance_id;

	// Primitive id (i.e. triangle) of the hit
	uint primitive_id;

	// Barycentric coordinates of the hit
	luisa::float2 barycentric;
	auto miss() const noexcept { return instance_id == ~0u; }
};

LUISA_STRUCT(RayCastHit, instance_id, primitive_id, barycentric)
{
	[[nodiscard]] auto miss() const noexcept { return instance_id == ~0u; }
};
