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
     * Gamma correct the color, all linear color from sRGB space should be gamma corrected before output
     * @param color Linear color
     * @param gamma Gamma value, default is 2.2
     * @return Gamma corrected color
     */
    FORCEINLINE Float3 gamma_correct(const Float3& color, const Float& gamma = 2.2f)
    {
        return pow(color, 1.f / gamma);
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