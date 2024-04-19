//
// Created by MarvelLi on 2024/4/17.
//

#pragma once
#include "luisa/luisa-compute.h"
namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;
    Float3 fresnel_schlick(const Float & cos_theta, const Float3 &f0)
    {
        return f0 + (1.f - f0) * pow(1.f - cos_theta, 5.f);
    }
}