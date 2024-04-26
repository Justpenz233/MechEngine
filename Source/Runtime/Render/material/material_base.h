//
// Created by MarvelLi on 2024/4/11.
//
#pragma once
#include <luisa/luisa-compute.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/TypeConvertion.h"
#include "Materials/Material.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

struct materialData
{
	uint material_type = ~0u;
	float3 base_color;
	float metalness;
	float roughness;
	float3 specular_tint;


	bool bUseTriangleNormal = false;
	bool bUseVertexNormal = true;

	materialData() = default;
	materialData(uint Tag, ::Material* InMaterial) :
	material_type(Tag),
	base_color(ToLuisaVector(InMaterial->BaseColor)),
	specular_tint(ToLuisaVector(InMaterial->SpecularTint)),
	metalness(InMaterial->Metalness),
	roughness(InMaterial->Roughness),
	bUseTriangleNormal(InMaterial->NormalType == FaceNormal),
	bUseVertexNormal(InMaterial->NormalType == VertexNormal)
	{}
};
}
LUISA_STRUCT(MechEngine::Rendering::materialData, material_type, base_color, metalness, roughness, specular_tint, bUseTriangleNormal, bUseVertexNormal){};


namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

struct material_parameters
{
	Float3 base_color;
	Float metalness;
	Float roughness;
	Float3 specular_tint;
	Float3 normal;
};

class material_base
{
public:
	material_base() = default;

	virtual ~material_base() = default;

	[[nodiscard]] Float3 shade(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const
	{
		material_parameters parameters{
			.base_color = sample_base_color(material_data, intersection, w, w_i),
			.metalness = sample_metalness(material_data, intersection, w, w_i),
			.roughness = sample_roughness(material_data, intersection, w, w_i),
			.specular_tint = sample_specular_tint(material_data, intersection, w, w_i),
			.normal = sample_normal(material_data, intersection, w, w_i)
		};
		return evaluate(parameters, intersection, w, w_i);
	}

protected:
	/**
	* Evaluate the material color at the given intersection point.
	* This should calculate the color of the material at the given point.
	* @param material_data  The material data for the current material, Contains textures and other material properties.
	* @param intersection  The intersection data for the surface point. Contatins the position, normal, uv.
	* @param w  The direction from the intersection point to the camera. Not normalized, contains the distance to the camera.
	* @param w_i The direction from the intersection point outside, as in BRDF.
	*/
	[[nodiscard]] virtual Float3 evaluate(const material_parameters& material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const = 0;

	/**
	 * Sample the diffuse property at the given intersection point.
	 * This give the material custom control over the diffuse color of the material.
	 * @return diffuse color of the material at the given intersection point.
	 */
	[[nodiscard]] virtual Float3 sample_base_color(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const { return material_data.base_color; }

	[[nodiscard]] virtual Float3 sample_specular_tint(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const { return material_data.specular_tint; }

	[[nodiscard]] virtual Float sample_metalness(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const { return material_data.metalness; }

	[[nodiscard]] virtual Float sample_roughness(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const { return material_data.roughness; }
	/**
	* Sample the normal at the given intersection point.
	* Will create shader variations based on the how the normal is sampled.
	*/
	[[nodiscard]] virtual Float3 sample_normal(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& w, const Float3& w_i) const
	{
		Float3 Normal;
		$if (material_data.bUseTriangleNormal)
		{
			Normal = intersection.triangle_normal_world;
		}
		$elif(material_data.bUseVertexNormal)
		{
			Normal = intersection.vertex_normal_world;
		}
		$else
		{
			Normal = make_float3(0.,0.,1.);
		};
		return Normal;
	}

};
}
