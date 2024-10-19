//
// Created by MarvelLi on 2024/4/25.
//

#pragma once

#include "light_base.h"
#include "Render/Core/sample.h"
#include "Render/RayTracing/RayTracingScene.h"
#include "Render/material/shading_function.h"

namespace MechEngine::Rendering
{
    class point_light : public light_base
    {
    public:
        using light_base::light_base;

    	virtual light_li_sample sample_li(Expr<light_data> data, const Float3& x, const Float2& u) const override
    	{
    		// Only consider the case when shading point outside the light source
    		auto light_transform = scene.get_instance_transform(data.instance_id);
    		const auto& radius = data.size.x;
    		auto center_obj = radius * sample_uniform_sphere_surface(u);
    		auto center_world = (scene.get_instance_transform(data.instance_id) * make_float4(center_obj, 1.f)).xyz();
    		// @see https://pbr-book.org/4ed/Shapes/Spheres
    		Float sinThetaMax = radius / distance(x, center_world);
    		Float sin2ThetaMax = square(sinThetaMax);
    		Float cosThetaMax = sqrt(1.f - sin2ThetaMax);
    		Float oneMinusCosThetaMax = 1.f - cosThetaMax;

    		// resample and calculate pdf
    		Float cosTheta = (cosThetaMax - 1.f) * u[0] + 1.f;
    		Float phi = u[1] * 2.f * pi;
    		Float sin2Theta = 1.f - square(cosTheta);
    		Float pdf = 1.f / (2.f * pi * (oneMinusCosThetaMax));

    		// calculate intersection point of sphere
    		Float cosAlpha = sin2Theta / sinThetaMax +
						cosTheta * sqrt(1.f - sin2Theta / square(sinThetaMax));
    		Float sinAlpha = sqrt(1.f - square(cosAlpha));
    		auto n = make_float3(sinAlpha * cos(phi), sinAlpha * sin(phi), cosAlpha);
    		auto p_l =  (light_transform * make_float4(radius * n, 1.f)).xyz();

    		auto l_i = data.intensity * data.light_color / distance_squared(x, p_l);
    		return {l_i, p_l - x, p_l, pdf};
    	}
    };


	class rectangle_light : public light_base
	{
	public:
		using light_base::light_base;

		virtual light_li_sample sample_li(Expr<light_data> data, const Float3& x, const Float2& u) const override
		{
			auto light_transform = scene.get_instance_transform(data.instance_id);
			const auto& size = data.size.xy();
			auto pdf = 1.f / (size.x * size.y);

			// resample from light source
			auto p_l_obj = make_float3((u.x - 0.5f) * size.x, 0.f, (u.y - 0.5f) * size.y);
			auto p_l = (light_transform * make_float4(p_l_obj, 1.f)).xyz();
			auto n_l = normalize((light_transform * make_float4(0.f, 0.f, 1.f, 0.f)).xyz());

			auto w_i = p_l - x;
			auto l_i = data.intensity * data.light_color * max(dot(normalize(w_i), n_l), 0.f) / distance_squared(x, p_l);
			return {l_i, w_i, p_l, pdf};
		}
	};
}
