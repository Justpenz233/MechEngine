//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "LightComponent.h"
#include "StaticMeshComponent.h"

MCLASS(PointLightComponent)
class ENGINE_API PointLightComponent : public LightComponent
{
	REFLECTION_BODY(PointLightComponent)
public:
	PointLightComponent() = default;
	~PointLightComponent() = default;

	FORCEINLINE double GetRadius() const { return Radius; }

protected:
	MPROPERTY(Slide_(0.1, 1.))
	double Radius = 0.1;

	virtual void BeginPlay() override;
	virtual void UploadRenderingData() override;
	void Remesh();
	virtual void PostEdit(Reflection::FieldAccessor& Field) override;


	// Used for a mesh representation of the light
	uint MeshId = ~0u;
	ObjectPtr<StaticMesh> MeshData;
};