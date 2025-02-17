//
// Created by MarvelLi on 2024/3/22.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "ActorComponent.h"
#include "Math/FTransform.h"


MCLASS(TransformComponent)
class ENGINE_API TransformComponent : public ActorComponent
{
	REFLECTION_BODY(TransformComponent)
	
public:
	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	virtual void TickComponent(double DeltaTime) override;

	void MarkDirty();
	FORCEINLINE Affine3d GetTransform() const;
	FORCEINLINE FTransform GetFTransform() const;
	FORCEINLINE FVector GetTranslation() const;
	FORCEINLINE FVector GetLocation() const;

	// Result in X,Y,Z , rotation order is x,y,z
	FORCEINLINE FVector GetRotationEuler() const;
	FORCEINLINE Quaterniond GetRotation() const;
	FORCEINLINE Matrix4d GetTransformMatrix() const;
	FORCEINLINE void SetTransform(const Affine3d& InTransform);
	FORCEINLINE void SetTransform(const FTransform& InTransfrom);
	FORCEINLINE void SetTranslation(const FVector& Translation);
	FORCEINLINE void AddTranslationGlobal(const FVector& DeltaTranslation);
	FORCEINLINE void AddTranslationLocal(const FVector& DeltaTranslation);

	void SetRotation(const FVector& InRotation);
	void AddRotationLocal(const FVector& DeltaRotation);
	void AddRotationGlobal(const FVector& DeltaRotation);

	FORCEINLINE void SetRotation(const Quaterniond& InRotation);
	FORCEINLINE void AddRotationLocal(const Quaterniond& DeltaRotation);
	FORCEINLINE void AddRotationGlobal(const Quaterniond& DeltaRotation);

	FORCEINLINE FVector GetScale() const;
	FORCEINLINE void SetScale(const FVector& InScale);
	FORCEINLINE void AddScale(const FVector& DeltaScale);

protected:
	MPROPERTY()
	FTransform Transform;

	void UploadRenderingData();

	bool bDirty = true;
};

FORCEINLINE Eigen::Affine3d TransformComponent::GetTransform() const
{
	return Eigen::Affine3d(GetTransformMatrix());
}

FORCEINLINE FTransform TransformComponent::GetFTransform() const
{
	return Transform;
}

FORCEINLINE Eigen::Matrix4d TransformComponent::GetTransformMatrix() const
{
	return Transform.GetMatrix();
}

FORCEINLINE Eigen::Vector3d TransformComponent::GetTranslation() const
{
	return Transform.GetLocation();
}

FORCEINLINE Eigen::Vector3d TransformComponent::GetLocation() const
{
	return Transform.GetLocation();
}

FORCEINLINE Eigen::Vector3d TransformComponent::GetRotationEuler() const
{
	return Transform.GetRotationEuler();
}

FORCEINLINE Eigen::Quaterniond TransformComponent::GetRotation() const
{
	return Transform.GetRotation();
}

FORCEINLINE void TransformComponent::SetRotation(const Eigen::Quaterniond &InRotation)
{
	MarkDirty();
	Transform.SetRotation(InRotation);
}

FORCEINLINE void TransformComponent::AddRotationLocal(const Eigen::Quaterniond &DeltaRotation)
{
	MarkDirty();
	Transform.AddRotationLocal(DeltaRotation);
}

FORCEINLINE void TransformComponent::AddRotationGlobal(const Eigen::Quaterniond &DeltaRotation)
{
	MarkDirty();
	Transform.AddRotationGlobal(DeltaRotation);
}

FORCEINLINE Eigen::Vector3d TransformComponent::GetScale() const
{
	return Transform.GetScale();
}

FORCEINLINE void TransformComponent::SetTransform(const Eigen::Affine3d& InTransform)
{
	MarkDirty();
	Transform = InTransform;
}

FORCEINLINE void TransformComponent::SetTransform(const FTransform& InTransform)
{
	MarkDirty();
	Transform = InTransform;
}

FORCEINLINE void TransformComponent::SetTranslation(const FVector& Translation)
{
	MarkDirty();
	Transform.SetTranslation(Translation);
}

FORCEINLINE void TransformComponent::AddTranslationGlobal(const FVector &DeltaTranslation)
{
	MarkDirty();
	Transform.AddTranslationGlobal(DeltaTranslation);
}

FORCEINLINE void TransformComponent::AddTranslationLocal(const FVector &DeltaTranslation)
{
	MarkDirty();
	Transform.AddTranslationLocal(DeltaTranslation);
}

FORCEINLINE void TransformComponent::SetScale(const FVector& InScale)
{
	MarkDirty();
	Transform.SetScale(InScale);
}
FORCEINLINE void TransformComponent::AddScale(const FVector &DeltaScale)
{
	MarkDirty();
	Transform.AddScale(DeltaScale);
}