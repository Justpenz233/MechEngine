//
// Created by MarvelLi on 2024/4/11.
//
#pragma once
#include <luisa/luisa-compute.h>
#include "ray_tracing_hit.h"
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
	float metalness;
};
}
LUISA_STRUCT(Rendering::materialData, material_type, diffuse, specular, metalness){};


namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

class material_base
{
protected:

	// Currently not support texture
	bool bHasDiffuseTexture = false;
	bool bHasSpecularTexture = false;
	bool bHasMetalnessTexture = false;
	bool bHasNormalTexture = false;

	bool bUseTriangleNormal = false;
	bool bUseVertexNormal = true;

public:
	material_base(Material* InMaterial)
	{
		if (InMaterial->NormalType == NormalMode::FaceNormal)
		{
			bUseTriangleNormal = true;
			bUseVertexNormal = false;
		}
		else if (InMaterial->NormalType == NormalMode::VertexNormal)
		{
			bUseTriangleNormal = false;
			bUseVertexNormal = true;
		}
	}
	/**
	* Evaluate the material color at the given intersection point.
	* This should calculate the color of the material at the given point.
	* @param material_data  The material data for the current material, Contains textures and other material properties.
	* @param intersection  The intersection data for the surface point. Contatins the position, normal, uv.
	* @param view_dir  The direction from the intersection point to the camera. Not normalized, contains the distance to the camera.
	* @param light_dir  The direction from the intersection point to the light source. Not normalized, contains the distance to the light.
	*/
	virtual Float3 evaluate(Expr<materialData> material_data, const ray_intersection& intersection, const Float3& view_dir, const Float3& light_dir) const = 0;

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
		if (bUseTriangleNormal)
			return intersection.triangle_normal_world;
		else if (bUseVertexNormal)
		{
			return intersection.vertex_normal_world;
		}
		else return make_float3(0.,0.,1.);
	}

};
}
