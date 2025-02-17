#pragma once
#include "JointParameter.h"
#include "Components/StaticMeshComponent.h"
#include "Core/CoreMinimal.h"
#include "Game/Actor.h"
#include "Object/Object.h"

// Kniematic joints, in tree hireachy, represent a actor in kniematic solvers
// Have no paramters in optimizing. 

class ENGINE_API Joint : public Object
{
protected:
    bool IsRoot = false;

public:
	// drive transform, used for drive, shoule be operated by IKSolver or tick function
	FTransform AddTransform = FTransform::Identity();
	FTransform InitTransform = FTransform::Identity();
	FTransform GlobalTransform;
	FTransform LocalTransform;

    WeakPtr<Joint>         ParentJoint;
	TArray<WeakPtr<Joint>> NextJoints;

    Joint(const FTransform& InInitTransform, bool InIsRoot = false)
    : InitTransform(InInitTransform), GlobalTransform(InitTransform), IsRoot(InIsRoot){}

    virtual void CalcGlobal();
    virtual void CalcLocal();

    virtual void AddLocation(const FVector& DriveLocation);
    virtual void AddRotation(const FQuat& DriveRotation);

    bool HasParent() const;
	ObjectPtr<Joint> GetParentJoint() const;

    bool IsRootJoint() const;
    virtual void SetIsRoot(bool InIsRoot);

    void AddNextJoint(ObjectPtr<Joint> NextJoint);


};
