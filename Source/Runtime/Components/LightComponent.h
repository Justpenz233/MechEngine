//
// Created by MarvelLi on 2024/4/5.
//
#pragma once

#include "RenderingComponent.h"
#include "Components/ActorComponent.h"
#include "Render/RendererInterface.h"

namespace MechEngine::Rendering {
class LightSceneProxy;
}

MCLASS(LightComponent)
class ENGINE_API LightComponent : public RenderingComponent
{
	friend class LightProxyBatch;

	REFLECTION_BODY(LightComponent)

public:
	LightComponent() = default;
	~LightComponent() = default;

	virtual void UploadRenderingData() override;

	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	/** Light intensity is only a scalar value representing the relative brightness of the light. */
	FORCEINLINE FVector GetIntensity() const;
	FORCEINLINE void SetIntensity(const FVector& InIntensity);

	FORCEINLINE FColor GetLightColor() const;
	FORCEINLINE void SetLightColor(const FColor& InColor);

	/** Get the total area of the light source */
	virtual float GetLightSourceTotalArea() const;
protected:
	MPROPERTY()
	FVector Intensity = FVector::Ones();

	MPROPERTY()
	FColor LightColor = FColor{1.0f, 1.0f, 1.0f};

protected:
	uint LightId = ~0u;
	uint InstanceId = ~0u;

};

FORCEINLINE void LightComponent::SetLightColor(const FColor& InColor)
{
	LightColor = InColor;
	MarkAsDirty();
}

FORCEINLINE void LightComponent::SetIntensity(const FVector& InIntensity)
{
	Intensity = InIntensity;
	MarkAsDirty();
}

FORCEINLINE FColor LightComponent::GetLightColor() const
{
	return LightColor;
}

FORCEINLINE void LightComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	MarkAsDirty();
}

FORCEINLINE FVector LightComponent::GetIntensity() const
{
	return Intensity;
}

