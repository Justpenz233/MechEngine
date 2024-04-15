//
// Created by MarvelLi on 2024/4/11.
//
#pragma once
#include <luisa/luisa-compute.h>
#include "ray_tracing_hit.h"
#include "TypeConvertion.h"
#include "Materials/Material.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

struct materialData
{
	uint material_type = ~0u;
	float3 diffuse;
	float3 specular;
	float metalness = 0.0f;

	bool bUseTriangleNormal = false;
	bool bUseVertexNormal = true;

	materialData() = default;
	materialData(uint Tag, Material* InMaterial) :
	material_type(Tag),
	diffuse(ToLuisaVector(InMaterial->Diffuse)),
	specular(ToLuisaVector(InMaterial->Specular)),
	metalness(InMaterial->Metalness),
	bUseTriangleNormal(InMaterial->NormalType == FaceNormal),
	bUseVertexNormal(InMaterial->NormalType == VertexNormal)
	{}
};
}
LUISA_STRUCT(Rendering::materialData, material_type, diffuse, specular, metalness, bUseTriangleNormal, bUseVertexNormal){};


namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

class material_base
{
protected:

public:
	material_base() = default;

	~material_base() = default;
	/**
	* Evaluate the material color at the given intersection point.
	* This should calculate the color of the material at the given point.
	* @param material_data  The material data for the current material, Contains textures and other material properties.
	* @param intersection  The intersection data for the surface point. Contatins the position, normal, uv.
	* @param view_dir  The direction from the intersection point to the camera. Not normalized, contains the distance to the camera.
	* @param light_dir  The direction from the intersection point to the light source. Not normalized, contains the distance to the light.
	*/
	[[nodiscard]] virtual Float3 evaluate(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) const = 0;

	/**
	 * Sample the diffuse property at the given intersection point.
	 * This give the material custom control over the diffuse color of the material.
	 * @return diffuse color of the material at the given intersection point.
	 */
	virtual Float3 sample_diffuse(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) { return material_data.diffuse; }

	virtual Float3 sample_specular(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) { return material_data.specular; }

	virtual Float sample_metalness(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) {return material_data.metalness; }

	/**
	* Sample the normal at the given intersection point.
	* Will create shader variations based on the how the normal is sampled.
	*/
	virtual Float3 sample_normal(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir)
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
