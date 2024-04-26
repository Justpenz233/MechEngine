//
// Created by MarvelLi on 2024/4/15.
//

#pragma once
#include "material_base.h"
#include "shading_function.h"

namespace MechEngine::Rendering
{
	class blinn_phong_material : public material_base
	{
		using material_base::material_base;


		[[nodiscard]] static Float D(const Float3& normal, const Float3& half, const Float& roughness)
		{
			auto shiness = lerp(0.1f, 10.f, (1.f - roughness));
			auto cos_theta_h = saturate(dot(normal, half));
			return (shiness + 2.f) / (2.f * pi) * pow(cos_theta_h, shiness);
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

			auto diffuse = material_data.base_color * (1.f - material_data.metalness) / pi;

			auto d = D(normal, half, material_data.roughness);
			// G = N dot L and N dot V but be normalized
			return d * 0.25f +  diffuse;
		}
	};
}
