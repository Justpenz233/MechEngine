//
// Created by MarvelLi on 2024/4/15.
//

#pragma once
#include "material_base.h"
namespace MechEngine::Rendering
{
	class blinn_phong_material : public material_base
	{
		using material_base::material_base;

		virtual Float3 evaluate(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) const override
		{
			auto normal = sample_normal(material_data, intersection, view_dir, light_dir);
			auto half_dir = normalize(light_dir + view_dir);
			auto diffuse = max(dot(normal, light_dir), 0.f) * material_data.diffuse;
			auto specular = pow(max(dot(normal, half_dir), 0.f), 32.f) * material_data.specular;
			return specular + diffuse;
		}
	};
}