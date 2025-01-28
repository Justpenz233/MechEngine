//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include "ray_intersection.h"
#include "luisa/luisa-compute.h"
#include "Render/Core/material_data.h"

namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    /**
     * A common render context, contains all render data needed for evaluate bxdf;
     */
    struct bxdf_context
    {
        const ray_intersection& intersection;
        Var<material_data> material_data;
    };
}
