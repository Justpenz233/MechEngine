//
// Created by Mayn on 2024/9/20.
//
#include "sample.h"


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
		auto theta = acos(1.f - 2.f * u.x);
		auto phi = 2.f * pi * u.y;
		auto sin_theta = sin(theta);
		return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta));
	};
	return impl(u);
}

Float3 sample_uniform_hemisphere_surface(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto theta = acos(1.f - 2.f * u.x) * 0.5f;
		auto phi = 2.f * pi * u.y;
		auto sin_theta = sin(theta);
		return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta));
	};
	return impl(u);
}
Float2 sample_uniform_disk(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto r = sqrt(u.x);
		auto theta = 2.f * pi * u.y;
		return make_float2(r * cos(theta), r * sin(theta));
	};
	return impl(u);
}

Float3 sample_cosine_hemisphere(Expr<float2> u) noexcept
{
	static Callable impl = [](const Float2& u) noexcept {
		auto theta = acos(sqrt(u.x));
		auto phi = 2.f * pi * u.y;
		auto sin_theta = sin(theta);
		return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta));
	};
	return impl(u);
}

Float pdf_cosine_hemisphere(Expr<float3> w) noexcept
{
	static Callable impl = [](const Float3& w) noexcept {
		return w.z * inv_pi;
	};
	return impl(w);
}

Float balance_heuristic(Expr<uint> nf, Expr<float> fPdf, Expr<uint> ng, Expr<float> gPdf) noexcept {
	static Callable impl = [](UInt nf, Float fPdf, UInt ng, Float gPdf) noexcept {
		auto sum_f = nf * fPdf;
		auto sum = sum_f + ng * gPdf;
		return ite(sum == 0.0f, 0.0f, sum_f / sum);
	};
	return impl(nf, fPdf, ng, gPdf);
}

Float power_heuristic(Expr<uint> nf, Expr<float> fPdf, Expr<uint> ng, Expr<float> gPdf) noexcept {
	static Callable impl = [](UInt nf, Float fPdf, UInt ng, Float gPdf) noexcept {
		Float f = nf * fPdf, g = ng * gPdf;
		auto ff = f * f;
		auto gg = g * g;
		auto sum = ff + gg;
		return ite(luisa::compute::isinf(ff), 1.f, ite(sum == 0.f, 0.f, ff / sum));
	};
	return impl(nf, fPdf, ng, gPdf);
}

Float balance_heuristic(Expr<float> fPdf, Expr<float> gPdf) noexcept {
	return balance_heuristic(1u, fPdf, 1u, gPdf);
}

Float power_heuristic(Expr<float> fPdf, Expr<float> gPdf) noexcept {
	return power_heuristic(1u, fPdf, 1u, gPdf);
}

};