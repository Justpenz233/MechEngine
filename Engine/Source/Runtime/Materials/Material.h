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
class Material : public Object
{
	REFLECTION_BODY(Material)
public:
	inline static FColor DefalutColor = FLinearColor(0.8, 0.8, 0.8);   //  1: Light Gray

	Material() = default;

	Material(const Material& Other) = default;

	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	FORCEINLINE void MarkAsDirty() { bDirty = true; }
	FORCEINLINE void ClearDirty() { bDirty = false; }
	FORCEINLINE bool IsDirty() const { return bDirty; }

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
	bool bDirty = false;
};

FORCEINLINE void Material::PostEdit(Reflection::FieldAccessor& Field)
{
	Object::PostEdit(Field);
	MarkAsDirty();
}

inline ObjectPtr<Material> Material::DefaultMaterial() {
	return NewObject<Material>();
}