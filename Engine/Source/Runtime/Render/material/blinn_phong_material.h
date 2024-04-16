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

		virtual Float3 evaluate(const material_parameters& material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) const override
		{
			auto normal = material_data.normal;
			auto half_dir = normalize(light_dir + view_dir);
			auto diffuse = material_data.base_color * (1.f - material_data.metalness);
			diffuse = max(dot(normal, light_dir), 0.f) * diffuse;
			auto specular = lerp(make_float3(0.04), material_data.base_color, material_data.metalness) * material_data.specular_tint;
			auto shininess = 100.f / (200.f * material_data.roughness + 0.01f);
			specular = pow(max(dot(normal, half_dir), 0.f), shininess) * specular;
			return specular + diffuse;
		}
	};
}