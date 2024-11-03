//
// Created by Mayn on 2024/9/20.
//

#pragma once

#include <luisa/dsl/syntax.h>

namespace MechEngine::Rendering
{
	using namespace luisa::compute;

	constexpr auto one_minus_epsilon = 0x1.fffffep-1f;

	inline Float uniform_uint_to_float(Expr<uint> u) noexcept {
		return min(one_minus_epsilon, u * 0x1p-32f);
	}

/**
 *  Linear Congruential Generator
 *	Generate a random float in [0, 1) using a linear congruential generator.
 *  @param state the state of the generator
 *  @return a random float in [0, 1)
 */
Float lcg(UInt &state) noexcept;


[[nodiscard]] UInt pcg(Expr<uint> v) noexcept;
[[nodiscard]] UInt2 pcg2d(Expr<uint2> v_in) noexcept;
[[nodiscard]] UInt3 pcg3d(Expr<uint3> v_in) noexcept;
[[nodiscard]] UInt4 pcg4d(Expr<uint4> v_in) noexcept;

[[nodiscard]] UInt xxhash32(Expr<uint> p) noexcept;
[[nodiscard]] UInt xxhash32(Expr<uint2> p) noexcept;
[[nodiscard]] UInt xxhash32(Expr<uint3> p) noexcept;
[[nodiscard]] UInt xxhash32(Expr<uint4> p) noexcept;

/**
 * Sample a point on in unit sphere uniformly.
 * @param u a random float2 in [0, 1]
 * @return a point on a unit sphere
 */
Float3 sample_uniform_sphere(Expr<float2> u) noexcept;

/**
 * Sample a point on sphere surface uniformly.
 * @param u a random float2 in [0, 1]
 * @return a point on the upper hemisphere
 */
Float3 sample_uniform_sphere_surface(Expr<float2> u) noexcept;

/**
 * Sample a point on upper hemisphere surface uniformly.
 * @param u a random float2 in [0, 1]
 * @return a point on the upper hemisphere
 */
Float3 sample_uniform_hemisphere_surface(Expr<float2> u) noexcept;


/**
 * Sample a point on disk uniformly.
 * @param u a random float2 in [0, 1]
 * @return a point on the disk
 */
Float2 sample_uniform_disk(Expr<float2> u) noexcept;

/**
 * Sample a point on hemisphere with cosine distribution.
 * @param u a random float2 in [0, 1]
 * @return a point on the hemisphere
 */
Float3 sample_cosine_hemisphere(Expr<float2> u) noexcept;

/**
 * Calculate the pdf of cosine hemisphere sampling.
 * @param w the direction to sample
 * @return the pdf of cosine hemisphere sampling
 */
Float pdf_cosine_hemisphere(Expr<float3> w) noexcept;


[[nodiscard]] Float balance_heuristic(Expr<uint> nf, Expr<float> fPdf, Expr<uint> ng, Expr<float> gPdf) noexcept;
[[nodiscard]] Float power_heuristic(Expr<uint> nf, Expr<float> fPdf, Expr<uint> ng, Expr<float> gPdf) noexcept;
[[nodiscard]] Float balance_heuristic(Expr<float> fPdf, Expr<float> gPdf) noexcept;
[[nodiscard]] Float power_heuristic(Expr<float> fPdf, Expr<float> gPdf) noexcept;
}