//
// Created by MarvelLi on 2024/4/11.
//
#pragma once
#include <luisa/luisa-compute.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/bxdf_context.h"
#include "Render/Core/material_data.h"


namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

struct material_parameters
{
	Float3 base_color;
	Float metalness;
	Float specular;
	Float roughness;
	Float3 specular_tint;
	Float3 normal;
};

class material_base
{
public:
	material_base() = default;

	virtual ~material_base() = default;

	/**
	 * Evaluate the BxDF of the material at the given intersection point.
	 * @param context  The context for the current bxdf evaluation.
	 * @param parameters The material parameters for the current material.
	 * @return The color of the material at the given intersection point.
	 */
	[[nodiscard]] Float3 bxdf(const bxdf_context& context, const material_parameters& parameters) const
	{
		return evaluate(parameters, context.intersection, context.w_o, context.w_i);
	}

	/**
	 * Calculate the material parameters for the current material. May have user defined calculation.
	 * @param context The context for the current bxdf evaluation.
	 * @return The material parameters for the current material.
	 */
	[[nodiscard]] material_parameters calc_material_parameters(const bxdf_context& context) const
	{
		return material_parameters{
			.base_color = sample_base_color(context),
			.specular = context.material_data.specular,
			.metalness = sample_metalness(context),
			.roughness = sample_roughness(context),
			.specular_tint = sample_specular_tint(context),
			.normal = sample_normal(context)
		};
	}


protected:
	/**
	* Evaluate the material color at the given intersection point.
	* This should calculate the color of the material at the given point.
	* @param material_data  The material data for the current material, Contains textures and other material properties.
	* @param intersection  The intersection data for the surface point. Contatins the position, normal, uv.
	* @param w_o The direction from the intersection point to the camera.  Output irradiance direction.
	* @param w_i The direction to the intersection point. Input irradiance direction.
	*/
	[[nodiscard]] virtual Float3 evaluate(const material_parameters& material_data, const ray_intersection& intersection, const Float3& w_o, const Float3& w_i) const = 0;

	/**
	 * Sample the diffuse property at the given intersection point.
	 * This give the material custom control over the diffuse color of the material.
	 * @return diffuse color of the material at the given intersection point.
	 */
	[[nodiscard]] virtual Float3 sample_base_color(const bxdf_context& context) const { return context.material_data.base_color; }

	[[nodiscard]] virtual Float3 sample_specular_tint(const bxdf_context& context) const { return context.material_data.specular_tint; }

	[[nodiscard]] virtual Float sample_metalness(const bxdf_context& context) const { return context.material_data.metalness; }

	[[nodiscard]] virtual Float sample_roughness(const bxdf_context& context) const { return context.material_data.roughness; }
	/**
	* Sample the normal at the given intersection point.
	* Will create shader variations based on the how the normal is sampled.
	*/
	[[nodiscard]] virtual Float3 sample_normal(const bxdf_context& context) const
	{
		Float3 Normal;
		$if (context.material_data.bUseTriangleNormal)
		{
			Normal = context.intersection.triangle_normal_world;
		}
		$elif(context.material_data.bUseVertexNormal)
		{
			Normal = context.intersection.vertex_normal_world;
		}
		$else
		{
			Normal = make_float3(0.,0.,1.);
		};
		return Normal;
	}
};
}
