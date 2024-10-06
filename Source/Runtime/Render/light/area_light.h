//
// Created by MarvelLi on 2024/4/25.
//

#pragma once

#include "light_base.h"
#include "Render/Core/random.h"
#include "Render/RayTracing/RayTracingScene.h"
#include "Render/material/shading_function.h"

namespace MechEngine::Rendering
{
    class point_light : public light_base
    {
    public:
        using light_base::light_base;

    	virtual light_li_sample sample_li(Expr<light_data> data, const Float3& x, const Float3& normal, const Float2& u) const override
    	{
    		const auto& radius = data.size.x;
    		auto p_obj = radius * sample_uniform_sphere(u);
    		auto p_world = (scene.get_instance_transform(data.instance_id) * make_float4(p_obj, 1.f)).xyz();
    		auto w_i = normalize(p_world - x);
    		auto pdf = 1.f / (4.f * pi * radius * radius);
    		auto l_i = data.intensity * data.light_color / distance_squared(x, p_world);
    		return {l_i, w_i, p_world, pdf};
    	}
    };
}
