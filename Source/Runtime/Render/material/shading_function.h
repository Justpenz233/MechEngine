//
// Created by MarvelLi on 2024/4/17.
//

#pragma once
#include "luisa/luisa-compute.h"
namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    FORCEINLINE Float3 fresnel_schlick(const Float& cos_theta, const Float3 &f0)
    {
        return f0 + (1.f - f0) * pow(1.f - cos_theta, 5.f);
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
}