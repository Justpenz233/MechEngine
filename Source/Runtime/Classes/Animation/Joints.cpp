#include "Joints.h"
#include "CoreMinimal.h"
#include "Game/World.h"

bool Joint::IsRootJoint() const
{
    return ParentJoint.expired() || IsRoot;
}

void Joint::SetIsRoot(bool InIsRoot)
{
    IsRoot = InIsRoot;
}

bool Joint::HasParent() const
{
    return !ParentJoint.expired();
}

void Joint::AddNextJoint(ObjectPtr<Joint> NextJoint)
{
	NextJoint->ParentJoint = Cast<Joint>(GetThis());
	NextJoints.push_back(NextJoint);
}

ObjectPtr<Joint> Joint::GetParentJoint() const
{
	return ParentJoint.lock();
}

void Joint::CalcGlobal()
{
	ASSERT(!IsRootJoint());
    if(HasParent())
    	GlobalTransform = GetParentJoint()->GlobalTransform * LocalTransform;

	GlobalTransform = GlobalTransform * AddTransform;
}

void Joint::CalcLocal()
{
	if (HasParent())
		LocalTransform = GetParentJoint()->InitTransform.Inverse() * InitTransform;
	else
		LocalTransform = FTransform::Identity();
}

void Joint::AddLocation(const FVector& DriveLocation)
{
	GlobalTransform.AddTranslationGlobal(DriveLocation);
}
void Joint::AddRotation(const FQuat& DriveRotation)
{
	GlobalTransform.AddRotationLocal(DriveRotation);
}