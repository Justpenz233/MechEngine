#include "IKJoint.h"
#include "Animation/Joints.h"
#include "Core/CoreMinimal.h"

Matrix4d IKEffectorTarget::GetTargetMatrix() const
{
	FTransform Result = FTransform::Identity();
	if(RotationEnabled)
		Result.SetRotation(Rotation);
	if(PositionEnabled)
		Result.SetTranslation(Position);
	return Result.GetMatrix();
}

IKJoint::IKJoint(EDOF3D InRoationDOF, EDOF3D InTranslationDOF,const FTransform& InInitTransform,  bool InIsRoot)
	: Joint(InInitTransform ,InIsRoot), RoatationDOF(InRoationDOF), TranslationDOF(InTranslationDOF), Parameter(InRoationDOF, InTranslationDOF)
{
	if(InIsRoot)
		SetDOF(FreeNone, FreeNone);
}

void IKJoint::SetIsRoot(bool InIsRoot)
{
	Joint::SetIsRoot(InIsRoot);
	if (InIsRoot)
		SetDOF(FreeNone, FreeNone);
}

void IKJoint::CalcGlobal()
{
	// Copy parameter to AddTransform
	if(ParameterNum() > 0)
		AddTransform = Parameter;
	Joint::CalcGlobal();
}

void IKJoint::SetTargetGlobalRotation(const FQuat& InTargetRotation, const FVector& InRotationStiffness)
{
	Target.RotationEnabled = true;
	Target.Rotation = InTargetRotation;
	RotationStiffness = InRotationStiffness;
}

void IKJoint::SetTargetTransform(const FTransform& InTargetTransform)
{
	Target.PositionEnabled = true;
	Target.RotationEnabled = true;
	Target.Position = InTargetTransform.GetTranslation();
	Target.Rotation = InTargetTransform.GetRotation();
}

Matrix4d IKJoint::CalcLoss() const
{
	if (!Target.Enable())
		return Matrix4d::Zero();

	Matrix4d TargetTransform = Target.GetTargetMatrix();
	Matrix4d Loss = GlobalTransform.GetMatrix() - TargetTransform;
	if (!Target.PositionEnabled)
	{
		for (int i = 0; i < 3; i++)
			Loss(i, 3) = 0.;
	}
	if (!Target.RotationEnabled)
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				Loss(i, j) = 0.;
	}
	return Loss;
}

void IKJoint::SetParameter(VectorXd& InParameter)
{
	Parameter.ReadParameter(InParameter);
	if(ParameterNum() > 0)
		AddTransform = Parameter;
}

VectorXd IKJoint::GetParameter() const
{
	return Parameter.GetParameter();
}

int IKJoint::ParameterNum() const
{
	return Parameter.ParameterNum();
}

void IKJoint::SetTargetGlobalLocation(const FVector& InTargetTranslation, const FVector& InLocationStiffness)
{
    Target.PositionEnabled = true;
    Target.Position = InTargetTranslation;
    LocationStiffness = InLocationStiffness;
}

void IKJoint::SetTargetOriginalLocation()
{
    Target.PositionEnabled = true;
    Target.Position = InitTransform.GetTranslation();
}

void IKJoint::SetTargetOriginalRotation()
{
    Target.RotationEnabled = true;
    Target.Rotation = InitTransform.GetRotation();
}

void IKJoint::SetTargetOriginalTransform()
{
    SetTargetOriginalLocation();
    SetTargetOriginalRotation();
}

bool IKJoint::IsSatisfyConstrain()
{
    bool Result = true;
    for(int i = 0;i < 3;i ++)
    {
        Result &= GetAddRotationEuler()(i) >= AddRotationRange[0][i];
        Result &= GetAddRotationEuler()(i) <= AddRotationRange[1][i];
        Result &= GetAddTranslation()(i) >= AddTranslationRange[0][i];
        Result &= GetAddTranslation()(i) <= AddTranslationRange[1][i];
    }
    return Result;
}

FVector IKJoint::GetAddRotationEuler() const
{
	return Parameter.GetRotationEuler();
}

FVector IKJoint::GetAddTranslation() const
{
	return Parameter.GetLocation();
}

bool IKJoint::IsEnableTarget() const
{
	return Target.Enable();
}
void IKJoint::SetDOF(EDOF3D InRotationDOF, EDOF3D InTranslationDOF)
{
	RoatationDOF = InRotationDOF;
	TranslationDOF = InTranslationDOF;
	Parameter = JointParameter(InRotationDOF, InTranslationDOF);
}