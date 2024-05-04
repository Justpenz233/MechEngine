//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include "Math/MathType.h"
#include "Object/Object.h"
#include "Game/World.h"
#include "Render/GPUSceneInterface.h"

enum MaterialMode
{
	BlinnPhong,
	Disney,
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
	inline static FColor DefalutColor = FLinearColor(1., 1., 1.);

	/**
	 * Whether to show the wireframe of the mesh.
	 */
	MPROPERTY()
	bool bShowWireframe = false;


	MPROPERTY()
	MaterialMode Mode = Disney;

	/**
	 * The normal type of the material.
	 * FaceNormal: Use the face normal as the normal.
	 * VertexNormal: Use the vertex normal as the normal.
	 * Will support normal map in the future.
	 */
	MPROPERTY()
	NormalMode NormalType = VertexNormal;

	/**
	 * Base color is used as diffuse color for non-metallic materials
	 * and as the specular color for metallic materials(because metal have no diffuse)
	 */
	MPROPERTY()
	FLinearColor BaseColor = DefalutColor;

	/**
	 * Specular tint is used to tint the specular reflection.
	 * This property is not physical based, but it's useful for artistic control.
	 */
	MPROPERTY()
	FLinearColor SpecularTint = {1.0f, 1.0f, 1.0f};

	/**
	 * Metalness is used to control the balance between diffuse and specular reflection.
	 * As math: specular_color = lerp(0.04, BaseColor, Metalness)
	 * 0.04: The default specular reflection of non-metallic materials is 4%.
	 * @see https://blog.selfshadow.com/publications/s2015-shading-course/hoffman/s2015_pbs_physics_math_slides.pdf
	 */
	MPROPERTY()
	float Metalness = 0.0f;

	MPROPERTY()
	float Specular = 0.5f;

	MPROPERTY()
	float Roughness = 0.5f;

	Material();

	Material(const Material& Other);

	FORCEINLINE void PostEdit(Reflection::FieldAccessor& Field) override;

	/**
	 * Should be called affter modifying the material properties.
	 */
	FORCEINLINE void UpdateMaterial();

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
	BaseColor = Other.BaseColor;
	SpecularTint = Other.SpecularTint;
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