//
// Created by Mayn on 2024/10/19.
//

#pragma once
#include <luisa/luisa-compute.h>

namespace MechEngine::Rendering
{
using namespace luisa::compute;

class frame {

private:
	Float3 _s;
	Float3 _t;
	Float3 _n;

public:
	frame() noexcept;
	frame(Expr<float3> s, Expr<float3> t, Expr<float3> n) noexcept;
	void flip() noexcept;
	[[nodiscard]] static frame make(Expr<float3> n) noexcept;
	[[nodiscard]] static frame make(Expr<float3> n, Expr<float3> s) noexcept;
	[[nodiscard]] Float3 local_to_world(Expr<float3> d) const noexcept;
	[[nodiscard]] Float3 world_to_local(Expr<float3> d) const noexcept;
	[[nodiscard]] Expr<float3> s() const noexcept { return _s; }
	[[nodiscard]] Expr<float3> t() const noexcept { return _t; }
	[[nodiscard]] Expr<float3> n() const noexcept { return _n; }
};

}