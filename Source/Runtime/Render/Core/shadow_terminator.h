//
// Created by MarvelLi on 2024/11/15.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "VertexData.h"

namespace MechEngine::Rendering
{
using namespace luisa::compute;

/**
 * Raytracing gem 2,  CHAPTER 4 HACKING THE SHADOW TERMINATOR
 * @param x local position
 * @param vps vertices of the triangle
 * @param bary barycentric coordinates of the intersection point
 * @return the shadow terminator position in local space
 */
inline Float3 shadow_terminator(const Float3& x, const ArrayVar<Vertex, 3>& vps, const Float2& bary)
{
	auto tmpu = x - vps[0]->position();
	auto tmpv = x - vps[1]->position();
	auto tmpw = x - vps[2]->position();

	auto dotu = min(0.f, dot(tmpu, vps[0]->normal()));
	auto dotv = min(0.f, dot(tmpv, vps[1]->normal()));
	auto dotw = min(0.f, dot(tmpw, vps[2]->normal()));

	tmpu -= dotu * vps[0]->normal();
	tmpv -= dotv * vps[1]->normal();
	tmpw -= dotw * vps[2]->normal();
	return x + triangle_interpolate(bary, tmpu, tmpv, tmpw);
}
};