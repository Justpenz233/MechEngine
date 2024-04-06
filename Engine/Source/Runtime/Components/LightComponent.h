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

class LightComponent : public RenderingComponent
{
	friend class LightProxyBatch;

	REFLECTION_BODY(LightComponent)

public:
	LightComponent() = default;
	~LightComponent() = default;

	virtual void BeginPlay() override;
	virtual void UploadRenderingData() override;

	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	FORCEINLINE float GetIntensity() const;
	FORCEINLINE void SetIntensity(float InIntensity);

	FORCEINLINE FColor GetLightColor() const;
	FORCEINLINE void SetLightColor(const FColor& InColor);

	FORCEINLINE void SetSamplesPerPixel(int InSamplesPerPixel);
	FORCEINLINE int GetSamplesPerPixel() const;

protected:
	MPROPERTY()
	float Intensity = 1.0f;

	MPROPERTY()
	FColor LightColor = FColor{1.0f, 1.0f, 1.0f};

	/** Samples per pixel for ray tracing */
	MPROPERTY()
	int SamplesPerPixel;

protected:
	bool bDirty = false;

};

FORCEINLINE void LightComponent::SetLightColor(const FColor& InColor)
{
	LightColor = InColor;
	MarkAsDirty();
}

FORCEINLINE void LightComponent::SetSamplesPerPixel(int InSamplesPerPixel) {
	SamplesPerPixel = InSamplesPerPixel;
	MarkAsDirty();
}

FORCEINLINE int LightComponent::GetSamplesPerPixel() const
{
	return SamplesPerPixel;
}

FORCEINLINE void LightComponent::SetIntensity(float InIntensity)
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

FORCEINLINE float LightComponent::GetIntensity() const
{
	return Intensity;
}

