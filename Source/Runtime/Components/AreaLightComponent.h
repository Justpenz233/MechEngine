//
// Created by Marveli on 2024/10/6.
//

#pragma once
#include "LightComponent.h"
#include "StaticMeshComponent.h"

MCLASS(AreaLightComponent)
class ENGINE_API AreaLightComponent : public LightComponent
{
	REFLECTION_BODY(AreaLightComponent)
public:
	AreaLightComponent() = default;
	~AreaLightComponent() = default;

	virtual void UploadRenderingData() override;
	void Remesh();
	virtual void PostEdit(Reflection::FieldAccessor& Field) override;


	FORCEINLINE FVector2 GetSize() const { return Size; }
	FORCEINLINE void SetSize(const FVector2& InSize) { Size = InSize; MarkAsDirty(); }

protected:
	MPROPERTY()
	FVector2 Size = {0.1, 0.1};

	// Used for a mesh representation of the light
	uint MeshId = ~0u;
	ObjectPtr<StaticMesh> MeshData;
};
