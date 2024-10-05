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

        [[nodiscard]] virtual Float3 l_i(Expr<light_data> data, const Float& distance, const Float3& w_i) const override
        {
            auto attenuation = 1.f / (distance * distance + data.radius);
            return data.intensity * srgb_to_linear(data.light_color) * attenuation;
        }
    };


	class const_point_light : public light_base
	{
	public:
	using light_base::light_base;

	[[nodiscard]] virtual Float3 l_i(Expr<light_data> data, const Float& distance, const Float3& w_i) const override
	{
		return data.intensity * srgb_to_linear(data.light_color);
	}
};
}
