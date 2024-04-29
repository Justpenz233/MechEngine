//
// Created by MarvelLi on 2024/4/29.
//

#pragma once
#include "g_buffer.h"
#include "ray_tracing_hit.h"
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
        Expr<Ray> ray;
        const ray_intersection& intersection;
        Float3 w_o;
        Float3 w_i;
        GBuffer& g_buffer;
        Var<materialData> material_data;
    };
}
