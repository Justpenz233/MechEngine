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


UInt xxhash32(Expr<uint> p) noexcept {
    static Callable impl = [](UInt p) noexcept {
        constexpr auto PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
        constexpr auto PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
        auto h32 = p + PRIME32_5;
        h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
        h32 = PRIME32_2 * (h32 ^ (h32 >> 15u));
        h32 = PRIME32_3 * (h32 ^ (h32 >> 13u));
        return h32 ^ (h32 >> 16u);
    };
    return impl(p);
}

UInt xxhash32(Expr<uint2> p) noexcept {
    static Callable impl = [](UInt2 p) noexcept {
        constexpr auto PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
        constexpr auto PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
        auto h32 = p.y + PRIME32_5 + p.x * PRIME32_3;
        h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
        h32 = PRIME32_2 * (h32 ^ (h32 >> 15u));
        h32 = PRIME32_3 * (h32 ^ (h32 >> 13u));
        return h32 ^ (h32 >> 16u);
    };
    return impl(p);
}

UInt xxhash32(Expr<uint3> p) noexcept {
    static Callable impl = [](UInt3 p) noexcept {
        constexpr auto PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
        constexpr auto PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
        UInt h32 = p.z + PRIME32_5 + p.x * PRIME32_3;
        h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
        h32 += p.y * PRIME32_3;
        h32 = PRIME32_4 * ((h32 << 17u) | (h32 >> (32u - 17u)));
        h32 = PRIME32_2 * (h32 ^ (h32 >> 15u));
        h32 = PRIME32_3 * (h32 ^ (h32 >> 13u));
        return h32 ^ (h32 >> 16u);
    };
    return impl(p);
}

UInt xxhash32(Expr<uint4> p) noexcept {
    static Callable impl = [](UInt4 p) noexcept {
        constexpr auto PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
        constexpr auto PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
        auto h32 = p.w + PRIME32_5 + p.x * PRIME32_3;
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

// https://www.pcg-random.org/
UInt pcg(Expr<uint> v) noexcept {
    static Callable impl = [](UInt v) noexcept {
        auto state = v * 747796405u + 2891336453u;
        auto word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    };
    return impl(v);
}

UInt2 pcg2d(Expr<uint2> v) noexcept {
    static Callable impl = [](UInt2 v) noexcept {
        v = v * 1664525u + 1013904223u;
        v.x += v.y * 1664525u;
        v.y += v.x * 1664525u;
        v = v ^ (v >> 16u);
        v.x += v.y * 1664525u;
        v.y += v.x * 1664525u;
        v = v ^ (v >> 16u);
        return v;
    };
    return impl(v);
}

// http://www.jcgt.org/published/0009/03/02/
UInt3 pcg3d(Expr<uint3> v) noexcept {
    static Callable impl = [](UInt3 v) noexcept {
        v = v * 1664525u + 1013904223u;
        v.x += v.y * v.z;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v ^= v >> 16u;
        v.x += v.y * v.z;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        return v;
    };
    return impl(v);
}

// http://www.jcgt.org/published/0009/03/02/
UInt4 pcg4d(Expr<uint4> v) noexcept {
    static Callable impl = [](UInt4 v) noexcept {
        v = v * 1664525u + 1013904223u;
        v.x += v.y * v.w;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v.w += v.y * v.z;
        v ^= v >> 16u;
        v.x += v.y * v.w;
        v.y += v.z * v.x;
        v.z += v.x * v.y;
        v.w += v.y * v.z;
        return v;
    };
    return impl(v);
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