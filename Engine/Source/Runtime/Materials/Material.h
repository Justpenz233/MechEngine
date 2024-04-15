//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include "Math/MathType.h"
#include "Object/Object.h"

enum MaterialMode
{
	BlinnPhong,
	CookTorrance,
	Disney,
	GGX
};
enum NormalMode
{
	FaceNormal,
	VertexNormal
};

/**
 * A base class for all materials.
 * Only contains the data for the material model.
 */
MCLASS(Material)
class Material : public Object
{
	REFLECTION_BODY(Material)
public:
	Material() = default;

	MPROPERTY()
	MaterialMode Model = BlinnPhong;

	MPROPERTY()
	NormalMode NormalType = VertexNormal;

	MPROPERTY()
	FLinearColor Diffuse;

	MPROPERTY()
	FLinearColor Specular;

	MPROPERTY()
	float Metalness = 0.0f;
};