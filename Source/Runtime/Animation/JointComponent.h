//
// Created by MarvelLi on 2023/12/20.
//

#pragma once
#include "IKJoint.h"
#include "Joints.h"
#include "Components/StaticMeshComponent.h"

MCLASS(JointComponent)
class ENGINE_API JointComponent : public StaticMeshComponent
{
	REFLECTION_BODY(JointComponent)
protected:
	JointComponent()= default;
	ObjectPtr<Joint> JointPtr;

public:
	// template Constructor forward all parameter to Joint

	explicit JointComponent(const FTransform& InInitTransform, bool InIsRoot = false);

	void Init() override;

	virtual void FeatchFromActor();
	virtual void ApplyToActor();

	virtual void Remesh() override;

	virtual ObjectPtr<StaticMesh> CreateJointMesh() { return nullptr; }

	ObjectPtr<Joint> GetJoint() {return JointPtr; }
};

MCLASS(IKJointComponent)
class ENGINE_API IKJointComponent : public JointComponent
{
	REFLECTION_BODY(IKJointComponent)
public:
	IKJointComponent() = default;
	IKJointComponent(EDOF3D InRoationDOF, EDOF3D InTranslationDOF, const FTransform& InInitTransform, bool InIsRoot = false);

	ObjectPtr<IKJoint> GetIKJoint() { return Cast<IKJoint>(JointPtr); }

	void SetTargetGlobalLocation(const FVector& InTargetTranslation, const FVector& InLocationStiffness = FVector::Ones());
	void SetTargetGlobalRotation(const FQuat& InTargetRotation, const FVector& InRotationStiffness = FVector::Ones());
	void SetTargetCurrentLocation();
	void SetTargetCurrentRotation();
	void SetTargetCurrentTransform();
	void SetTargetTransform(const FTransform& InTargetTransform);

	void AddNextJoint(ObjectPtr<IKJointComponent> NextJoint);
};
