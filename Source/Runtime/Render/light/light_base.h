//
// Created by MarvelLi on 2024/4/25.
//

#pragma once
#include "luisa/luisa-compute.h"

namespace MechEngine::Rendering
{
    struct light_data
    {
        //----- Base data for all the lights -------//
        luisa::uint light_type = ~0u;

        luisa::uint transform_id = ~0u;

        float intensity = 1.f;

        // light color in linear space
        luisa::float3 light_color;

        //radius for area light(in disk) and point light
        float radius{};
    };
}

LUISA_STRUCT(MechEngine::Rendering::light_data, light_type, transform_id, intensity, light_color, radius) {};


namespace MechEngine::Rendering
{
    using namespace luisa;
    using namespace luisa::compute;

    /**
     * A common light data structure for all lights.
     * Now support point light, spot light, directional light, area light
     */

    class light_base
    {
    public:

        light_base() = default;

        /**
         * Evaluate the light irradiance at hit_pos in the direction of w_i
         * @param data light data used to evaluate the light
         * @param light_transform the transform of the light, used to transform the light position
         * @param x the position to evaluate the light, same as render equation
         * @param w_i the direction to evaluate the light, same as render equation
         * @return the light irradiance at x in the direction of w_i
         */
        [[nodiscard]] virtual Float3 l_i(Expr<light_data> data, const Float4x4& light_transform, const Float3& x, const Float3& w_i) const = 0;

    };
}