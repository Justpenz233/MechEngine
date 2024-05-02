//
// Created by MarvelLi on 2024/4/25.
//

#pragma once

#include "light_base.h"
#include "Render/material/shading_function.h"

namespace MechEngine::Rendering
{
    class point_light : public light_base
    {
    public:
        using light_base::light_base;

        [[nodiscard]] virtual Float3 l_i(Expr<light_data> data, const Float4x4& light_transform, const Float3& x, const Float3& w_i) const override
        {
            auto light_pos = light_transform[3].xyz();
            auto distance = clamp(length(x - light_pos), 0.01f, 1000.f);
            auto attenuation = 1.f / (distance * distance);
            return data.intensity * srgb_to_linear(data.light_color) * attenuation;
        }
    };
}
