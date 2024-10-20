//
// Created by MarvelLi on 2024/3/28.
//


#pragma once
#include "frame.h"
#include "shape.h"

namespace MechEngine::Rendering
{
using namespace luisa::compute;
struct ray_tracing_hit
{
	// Instance id of the hit
	UInt instance_id;

	// Primitive id (i.e. triangle) of the hit
	UInt primitive_id;

	// Barycentric coordinates of the hit
	Float2 barycentric;

	[[nodiscard]] auto miss() const noexcept { return instance_id == ~0u; }
};
using hit = ray_tracing_hit;

struct ray_intersection
{
	Float3 triangle_normal_world;
	Float3 vertex_normal_world;
	Float3 corner_normal_world;
	Float depth;

	Float3 position_world;
	Float2 barycentric;
	Float2 uv;

	Var<shape> shape;
	UInt instance_id;
	UInt primitive_id;
	UInt material_id;
	Bool back_face;

	frame shading_frame;

	ray_intersection(): instance_id(~0u) {}
	[[nodiscard]] auto valid() const noexcept { return instance_id != ~0u; }

};
}