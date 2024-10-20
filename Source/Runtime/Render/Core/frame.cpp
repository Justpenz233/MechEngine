//
// Created by Mayn on 2024/10/19.
//

#include "frame.h"
#include "math_function.h"

namespace MechEngine::Rendering
{
frame::frame(Expr<float3> s,
			 Expr<float3> t,
			 Expr<float3> n) noexcept
	: _s{s}, _t{t}, _n{n} {}

frame::frame() noexcept
	: _s{make_float3(1.f, 0.f, 0.f)},
	  _t{make_float3(0.f, 1.f, 0.f)},
	  _n{make_float3(0.f, 0.f, 1.f)} {}

frame frame::make(Expr<float3> n) noexcept {
	auto sgn = sign(n.z);
	auto a = -1.f / (sgn + n.z);
	auto b = n.x * n.y * a;
	auto s = make_float3(1.f + sgn * n.x * n.x * a, sgn * b, -sgn * n.x);
	auto t = make_float3(b, sgn + n.y * n.y * a, -n.y);
	return {normalize(s), normalize(t), n};
}

frame frame::make(Expr<float3> n, Expr<float3> s) noexcept {
	auto ss = normalize(s - n * dot(n, s));
	auto tt = normalize(cross(n, ss));
	return {ss, tt, n};
}

Float3 frame::local_to_world(Expr<float3> d) const noexcept {
	return normalize(d.x * _s + d.y * _t + d.z * _n);
}

Float3 frame::world_to_local(Expr<float3> d) const noexcept {
	return normalize(make_float3(dot(d, _s), dot(d, _t), dot(d, _n)));
}

void frame::flip() noexcept {
	_n = -_n;
	_t = -_t;
}
}