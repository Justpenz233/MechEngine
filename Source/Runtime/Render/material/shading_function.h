//
// Created by MarvelLi on 2024/4/17.
//

#pragma once
#include "luisa/luisa-compute.h"
namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    template<class T>
    FORCEINLINE T pow5(const T& x)
    {
        auto x2 = x * x;
        return x2 * x2 * x;
    }

    /**
     * Calculate the fresnel term using Schlick's approximation
     * @param f0 Base color of the material, should be interpolated between 0.04 and the base color by metalness
     * @param cos_theta Cosine of the angle between the normal and the half vector
     * @param f90 The color when the view is perpendicular to the surface, default is 1
     * @return Fresnel term
     */
    FORCEINLINE Float3 fresnel_schlick(const Float3 &f0, const Float& cos_theta, const Float3& f90 = make_float3(1.f))
    {
        return f0 + (f90 - f0) * pow5(1.f - cos_theta);
    }

    FORCEINLINE Float3 fresnel_schlick(const Float3& f0, const Float3& w_o, const Float3& h, const Float3& f90 = make_float3(1.f))
    {
        return f0 + (f90 - f0) * pow5(1.f - saturate(dot(w_o, h)));
    }

    /**
     * Convert linear color to sRGB color space
     * @param x Linear color
     * @return sRGB color
     */
    FORCEINLINE auto linear_to_srgb(const Float3& x) noexcept
    {
        return saturate(select(1.055f * pow(x, 1.0f / 2.4f) - 0.055f,
                               12.92f * x,
                               x <= 0.00031308f));
    }

    FORCEINLINE auto srgb_to_linear(const Float3& x) noexcept
    {
        return select(pow((x + 0.055f) / 1.055f, 2.4f),
                      x / 12.92f,
                      x <= 0.04045f);
    }

	FORCEINLINE auto srgb_to_linear(const float3& x) noexcept
    {
    	return float3{
    		(x.x <= 0.04045f) ? x.x / 12.92f : std::pow((x.x + 0.055f) / 1.055f, 2.4f),
			(x.y <= 0.04045f) ? x.y / 12.92f : std::pow((x.y + 0.055f) / 1.055f, 2.4f),
			(x.z <= 0.04045f) ? x.z / 12.92f : std::pow((x.z + 0.055f) / 1.055f, 2.4f)
		};
    }


	//@ref https://www.zhihu.com/question/5108732766/answer/49973544359
	[[nodiscard]] inline auto srgb_to_acescg(const float3& col)
	{
		constexpr auto mat = transpose(float3x3(
			float3(0.61319f, 0.33951f, 0.04737f),
			float3(0.07021f, 0.91634f, 0.01345f),
			float3(0.02062f, 0.10957f, 0.86961f)));
		return mat * col;
	}

	inline auto acescg_to_srgb(Expr<float3> col) {
    	constexpr auto mat = transpose(float3x3(
			float3(1.70505, -0.62179, -0.08326),
			float3(-0.13026, 1.14080, -0.01055),
			float3(-0.02400, -0.12897, 1.15297)));
    	return mat * col;
    }

    /**
     * Calculate the half vector of two vectors
     * @return Half vector
     */
    FORCEINLINE Float3 half_vector(const Float3& x, const Float3& y)
    {
        return normalize(x + y);
    }
}