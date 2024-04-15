//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include "Math/MathType.h"
#include "Object/Object.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"

enum MaterialMode
{
	BlinnPhong,
	CookTorrance,
	Disney,
	GGX,
	Custom // should provide a shader pointer
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
class Material final : public Object
{
	REFLECTION_BODY(Material)
public:
	inline static FColor DefalutColor = FLinearColor(0.8, 0.8, 0.8);   //  1: Light Gray

	Material();

	Material(const Material& Other);

	FORCEINLINE void PostEdit(Reflection::FieldAccessor& Field) override;

	/**
	 * Should be called affter modifying the material properties.
	 */
	FORCEINLINE void UpdateMaterial();

	MPROPERTY()
	MaterialMode Mode = BlinnPhong;

	MPROPERTY()
	NormalMode NormalType = VertexNormal;

	MPROPERTY()
	FLinearColor Diffuse = DefalutColor;

	MPROPERTY()
	FLinearColor Specular = DefalutColor;

	MPROPERTY()
	float Metalness = 0.0f;

	static ObjectPtr<Material> DefaultMaterial();

protected:
	/**
	* Register the material to the renderer. Should be called affter creating the material.
	*/
	FORCEINLINE void RegisterMaterial();
};

inline Material::Material()
{
	RegisterMaterial();
}

inline Material::Material(const Material& Other)
 : Object(Other) {
	Mode = Other.Mode;
	NormalType = Other.NormalType;
	Diffuse = Other.Diffuse;
	Specular = Other.Specular;
	Metalness = Other.Metalness;
	RegisterMaterial();
}

FORCEINLINE void Material::PostEdit(Reflection::FieldAccessor& Field) {
	UpdateMaterial();
}

FORCEINLINE void Material::RegisterMaterial() {
	GWorld->GetScene()->AddMaterial(this);
}

FORCEINLINE void Material::UpdateMaterial() {
	GWorld->GetScene()->UpdateMaterial(this);
}

inline ObjectPtr<Material> Material::DefaultMaterial() {
	return NewObject<Material>();
}