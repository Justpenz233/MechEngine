//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include "Math/MathType.h"
#include "Object/Object.h"
#include "Game/World.h"

namespace MechEngine::Rendering
{
class MaterialSceneProxy;
struct materialData;
}
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
class ENGINE_API Material final : public Object
{
	REFLECTION_BODY(Material)

public:
	Material();

	Material(const Material& Other);

	FORCEINLINE void SetBaseColor(const FColor& InColor);
	FORCEINLINE void SetShowWireframe(bool bShow);
	FORCEINLINE void SetMode(MaterialMode InMode);
	FORCEINLINE void SetNormalType(NormalMode InType);
	FORCEINLINE void SetSpecularTint(const FColor& InColor);
	FORCEINLINE void SetMetalness(float InMetalness);
	FORCEINLINE void SetSpecular(float InSpecular);
	FORCEINLINE void SetRoughness(float InRoughness);


	FORCEINLINE void PostEdit(Reflection::FieldAccessor& Field) override;

	/**
	 * Should be called affter modifying the material properties.
	 */
	void UpdateMaterial();

	static ObjectPtr<Material> DefaultMaterial();

protected:
	friend class MechEngine::Rendering::materialData;
	friend class MechEngine::Rendering::MaterialSceneProxy;
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

	/**
	* Register the material to the renderer. Should be called affter creating the material.
	*/
	void RegisterMaterial();
};

inline Material::Material(const Material& Other)
 : Object(Other) {
	Mode = Other.Mode;
	NormalType = Other.NormalType;
	BaseColor = Other.BaseColor;
	SpecularTint = Other.SpecularTint;
	Metalness = Other.Metalness;
	RegisterMaterial();
}

FORCEINLINE void Material::SetBaseColor(const FColor& InColor) {
	BaseColor = InColor;
	UpdateMaterial();
}

FORCEINLINE void Material::SetShowWireframe(bool bShow) {
	bShowWireframe = bShow;
	UpdateMaterial();
}

FORCEINLINE void Material::SetMode(MaterialMode InMode) {
	Mode = InMode;
	UpdateMaterial();
}

FORCEINLINE void Material::SetNormalType(NormalMode InType) {
	NormalType = InType;
	UpdateMaterial();
}

FORCEINLINE void Material::SetSpecularTint(const FColor& InColor) {
	SpecularTint = InColor;
	UpdateMaterial();
}

FORCEINLINE void Material::SetMetalness(float InMetalness) {
	Metalness = InMetalness;
	UpdateMaterial();
}

FORCEINLINE void Material::SetSpecular(float InSpecular) {
	Specular = InSpecular;
	UpdateMaterial();
}

FORCEINLINE void Material::SetRoughness(float InRoughness) {
	Roughness = InRoughness;
	UpdateMaterial();
}

FORCEINLINE void Material::PostEdit(Reflection::FieldAccessor& Field) {
	UpdateMaterial();
}

inline ObjectPtr<Material> Material::DefaultMaterial() {
	return NewObject<Material>();
}