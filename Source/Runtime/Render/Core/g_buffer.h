//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include <luisa/runtime/image.h>

namespace MechEngine::Rendering
{
struct GBuffer
{
    luisa::compute::Image<float> base_color;
    luisa::compute::Image<float> normal;
    luisa::compute::Image<float> depth;
    luisa::compute::Image<uint> instance_id;
    luisa::compute::Image<uint> material_id;
};
};