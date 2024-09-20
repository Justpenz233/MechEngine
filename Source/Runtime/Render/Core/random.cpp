//
// Created by Mayn on 2024/9/20.
//
#include "random.h"


namespace MechEngine::Rendering
{

Float lcg(UInt& state) noexcept
{
	static Callable impl = [](UInt& state) noexcept {
		constexpr auto lcg_a = 1664525u;
		constexpr auto lcg_c = 1013904223u;
		state = lcg_a * state + lcg_c;
		return uniform_uint_to_float(state);
	};
	return impl(state);
}

UInt xxhash32(Expr<uint4> p) noexcept
{
	static Callable impl = [](UInt4 p) noexcept {
		constexpr auto PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
		constexpr auto PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
		auto		   h32 = p.w + PRIME32_5 + p.x * PRIME32_3;
		h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
		h32 += p.y * PRIME32_3;
		h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
		h32 += p.z * PRIME32_3;
		h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
		h32 = PRIME32_2 * (h32 ^ (h32 >> 15u));
		h32 = PRIME32_3 * (h32 ^ (h32 >> 13u));
		return h32 ^ (h32 >> 16u);
	};
	return impl(p);
}

Float3 sample_uniform_sphere(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto z = 1.0f - 2.0f * u.x;
		auto r = sqrt(max(1.0f - z * z, 0.0f));
		auto phi = 2.0f * pi * u.y;
		return make_float3(r * cos(phi), r * sin(phi), z);
	};
	return impl(u);
}

Float3 sample_uniform_sphere_surface(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto theta = acos(1. - 2. * u.x);
		auto phi = 2. * pi * u.y;
		auto sin_theta = sin(theta);
		return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta));
	};
	return impl(u);
}

Float3 sample_uniform_hemisphere_surface(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto theta = acos(1.f - 2.f * u.x) * 0.5f;
		auto phi = 2. * pi * u.y;
		auto sin_theta = sin(theta);
		return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta));
	};
	return impl(u);
}

};