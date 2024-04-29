//
// Created by MarvelLi on 2024/4/26.
//

#pragma once

#include "material_base.h"
#include "shading_function.h"

namespace MechEngine::Rendering
{
    /**
     * Disney's physically based shading model
     * @see https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
     */
    class disney_material : public material_base
    {
        using material_base::material_base;

        [[nodiscard]] static Float D(const Float3& normal, const Float3& half, const Float& roughness)
        {

        }

        static Float3 disney_f90(const Float& roughness, const Float3& w_o, const Float3& half)
        {
            return 0.5f + 2.f * roughness * pow(dot(half, w_o), 2.f);
        }

        /**
         * Phsiacally based blinn-phong shading model, not normalized
         * @see https://www.zhihu.com/question/48050245/answer/108902674
         */
        [[nodiscard]]
        virtual Float3 evaluate(const material_parameters& material_data, const ray_intersection& intersection, const Float3& w_o, const Float3& w_i) const override
        {
            auto normal = material_data.normal;
            auto half = normalize(w_i + w_o);

            auto NdotL = dot(normal, w_i);
            auto NdotV = dot(normal, w_o);
            $if (NdotL < 0 | NdotV < 0)
            {
                return make_float3(0.);
            };

        }
    };
}
