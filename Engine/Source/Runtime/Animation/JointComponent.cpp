//
// Created by MarvelLi on 2023/12/20.
//

#include "JointComponent.h"
#include "Game/Actor.h"
#include "Mesh/BasicShapesLibrary.h"

JointComponent::JointComponent( const FTransform& InInitTransform, bool InIsRoot)
	:JointPtr(NewObject<Joint>(InInitTransform, InIsRoot)) {}

void JointComponent::Init()
{
    ActorComponent::Init();
	MarkAsDirty(DIRTY_ALL);
	// We did not init mesh here,
	// cause usually at this moment the nextjoint is not setup
}

void JointComponent::FeatchFromActor()
{
	JointPtr->GlobalTransform = Owner->GetFTransform();
}

void JointComponent::ApplyToActor()
{
	Owner->SetTransform(JointPtr->GlobalTransform);
}

void JointComponent::Remesh()
{
	StaticMeshComponent::Remesh();
	SetMeshData(CreateJointMesh());
}

IKJointComponent::IKJointComponent(EDOF3D InRoationDOF, EDOF3D InTranslationDOF, const FTransform& InInitTransform, bool InIsRoot)
	: JointComponent()
{
	JointPtr = NewObject<IKJoint>(InRoationDOF, InTranslationDOF, InInitTransform, InIsRoot);
}

void IKJointComponent::SetTargetGlobalLocation(const FVector& InTargetTranslation, const FVector& InLocationStiffness)
{
	GetIKJoint()->SetTargetGlobalLocation(InTargetTranslation, InLocationStiffness);
}

void IKJointComponent::SetTargetGlobalRotation(const FQuat& InTargetRotation, const FVector& InRotationStiffness)
{
	GetIKJoint()->SetTargetGlobalRotation(InTargetRotation, InRotationStiffness);
}

void IKJointComponent::SetTargetCurrentLocation()
{
	GetIKJoint()->SetTargetGlobalLocation(GetOwner()->GetTranslation());
}

void IKJointComponent::SetTargetCurrentRotation()
{
	GetIKJoint()->SetTargetGlobalRotation(GetOwner()->GetRotation());
}

void IKJointComponent::SetTargetCurrentTransform()
{
	GetIKJoint()->SetTargetGlobalLocation(GetOwner()->GetTranslation());
	GetIKJoint()->SetTargetGlobalRotation(GetOwner()->GetRotation());
}
void IKJointComponent::SetTargetTransform(const FTransform& InTargetTransform)
{
	GetIKJoint()->SetTargetTransform(InTargetTransform);
}

void IKJointComponent::AddNextJoint(ObjectPtr<IKJointComponent> NextJoint)
{
	GetIKJoint()->AddNextJoint(NextJoint->GetIKJoint());
}
