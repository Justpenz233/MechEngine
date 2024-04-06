//
// Created by MarvelLi on 2024/3/22.
//

#include "TransformComponent.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"


void TransformComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	ActorComponent::PostEdit(Field);
	if(Field.GetName() == NAME(Transform))
	{
		MarkDirty();
	}
}

void TransformComponent::TickComponent(double DeltaTime)
{
	ActorComponent::TickComponent(DeltaTime);
	if(bDirty)
	{
		UploadRenderingData();
		bDirty = false;
	}
}

void TransformComponent::MarkDirty()
{
	bDirty = true;
	GetOwner()->GetTransformUpdateDelegate().Broadcast();
}

void TransformComponent::SetRotation(const FVector& InRotation)
{
	MarkDirty();
	FQuat Rotation = AngleAxisd(InRotation.z(), Vector3d::UnitZ())
    * AngleAxisd(InRotation.y(), Vector3d::UnitY())
    * AngleAxisd(InRotation.x(), Vector3d::UnitX());
	Transform.SetRotation(Rotation);
}


void TransformComponent::AddRotationLocal(const FVector& DeltaRotation)
{
	MarkDirty();
	Quaterniond P = AngleAxisd(DeltaRotation.z(), Vector3d::UnitZ())
    * AngleAxisd(DeltaRotation.y(), Vector3d::UnitY())
    * AngleAxisd(DeltaRotation.x(), Vector3d::UnitX());
	AddRotationLocal(P);
}

void TransformComponent::AddRotationGlobal(const FVector& DeltaRotation)
{
	MarkDirty();
	FQuat P = AngleAxisd(DeltaRotation.z(), Vector3d::UnitZ())
    * AngleAxisd(DeltaRotation.y(), Vector3d::UnitY())
    * AngleAxisd(DeltaRotation.x(), Vector3d::UnitX());
	AddRotationGlobal(P);
}

void TransformComponent::UploadRenderingData()
{
	World->GetScene()->UpdateTransform(this);
}