//
// Created by MarvelLi on 2024/3/28.
//


#pragma once
#include "shape.h"

namespace MechEngine::Rendering
{
using namespace luisa::compute;

using hit = RayCastHit;

struct ray_intersection
{
	Float3 triangle_normal_world;
	Float3 vertex_normal_world;
	Float3 corner_normal_world;
	Float depth;

	Float3 position_world;
	Float2 barycentric;
	Float2 uv;
	Float2 motion_vector;

	Var<shape> shape;
	UInt instance_id;
	UInt primitive_id;
	UInt material_id;
	Bool back_face;

	ray_intersection(): instance_id(~0u) {}
	[[nodiscard]] auto valid() const noexcept { return instance_id != ~0u; }

};
};