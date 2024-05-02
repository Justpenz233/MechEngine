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

    /**
     * Calculate the half vector of two vectors
     * @return Half vector
     */
    FORCEINLINE Float3 half_vector(const Float3& x, const Float3& y)
    {
        return normalize(x + y);
    }
}