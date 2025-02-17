#pragma once
#include "JointParameter.h"
#include "Core/CoreMinimal.h"
#include "Joints.h"

struct IKEffectorTarget
{
	bool TargetJointEnable = false;
	WeakObjectPtr<Joint> TargetJoint;

    bool PositionEnabled = false;
    FVector Position;

    bool RotationEnabled = false;
    FQuat Rotation;

    bool Enable() const
    { return PositionEnabled | RotationEnabled; }

	Matrix4d GetTargetMatrix() const;

	FVector GetTargetLocation() const
	{
		if (TargetJointEnable)
			return TargetJoint.lock()->GlobalTransform.GetTranslation();
		else
			return Position;
	}

	FQuat GetTargetRotation() const
	{
		if (TargetJointEnable)
			return TargetJoint.lock()->GlobalTransform.GetRotation();
		else
			return Rotation;
	}
};

class ENGINE_API IKJoint : public Joint
{
protected:
    FVector RotationStiffness = FVector::Ones();
    FVector LocationStiffness = FVector::Ones();
    IKEffectorTarget Target;
	JointParameter Parameter;

	EDOF3D RoatationDOF;
	EDOF3D TranslationDOF;
public:
	IKJoint(EDOF3D InRoationDOF, EDOF3D InTranslationDOF, const FTransform& InInitTransform, bool InIsRoot = false);

	virtual void SetIsRoot(bool InIsRoot) override;
	/// \brief Apply parameters to AddTransform Then calc GlobalTransform.
	virtual void CalcGlobal() override;

    FVector AddRotationRange[2] = 
    {FVector::Constant(std::numeric_limits<double>::lowest()), FVector::Constant(std::numeric_limits<double>::max())};

    FVector AddTranslationRange[2] = 
    {FVector::Constant(std::numeric_limits<double>::lowest()), FVector::Constant(std::numeric_limits<double>::max())};

    ObjectPtr<IKJoint> GetParentJoint() { return Cast<IKJoint>(ParentJoint.lock()); }

public:
	/// \brief Calc loss to target transform, in matrix 4d, should discard the last row in further processing
	/// \return Loss in Matrix4d
	Matrix4d CalcLoss() const;

	/// \brief Set parameter for this joint, from ik solver
	void SetParameter(VectorXd& InParameter);

	VectorXd GetParameter() const;

	int ParameterNum() const;
    void SetTargetGlobalLocation(const FVector& InTargetTranslation, const FVector& InLocationStiffness = FVector::Ones());
    void SetTargetGlobalRotation(const FQuat& InTargetRotation, const FVector& InRotationStiffness = FVector::Ones());
	void SetTargetTransform(const FTransform& InTargetTransform);

	void SetTargetCurrentLocation();
	void SetTargetCurrentRotation();
	void SetTargetCurrentTransform();
	void SetTargetOriginalLocation();
    void SetTargetOriginalRotation();
    void SetTargetOriginalTransform();
    bool IsSatisfyConstrain();
    FVector GetAddRotationEuler() const;
    FVector GetAddTranslation() const;

    bool IsEnableTarget() const;

	// Set dof for rotation and translation
	void SetDOF(EDOF3D InRotationDOF, EDOF3D InTranslationDOF);
};