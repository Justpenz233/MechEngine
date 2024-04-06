//
// Created by MarvelLi on 2023/12/19.
//

#include "JointParameter.h"
#include "Math/LinearAlgebra.h"
JointLocationParamter::operator FVector&()
{
	return Location;
}
JointLocationParamter::operator FVector()
{
	return Location;
}

Matrix4d JointLocationParamter::MakeMatrix4d()
{
	Matrix4d Result = Matrix4d::Identity();
	for(int i = 0; i < 3; i ++)
		Result(i, 3) = Location[i];
	return Result;
}

inline int JointLocationParamter::ParameterNum() const
{
	static std::map<EDOF3D, int> t{ { FreeNone, 0 }, { FreeX, 1 }, { FreeY, 1 }, { FreeZ, 1 },
		{ FreeXY, 2 }, { FreeXZ, 2 }, { FreeYZ, 2 }, { FreeXYZ, 3 } };
	return t[DOF];
}

inline void JointLocationParamter::ReadParameter(VectorXd& Parameter)
{
	int ParameterCount = ParameterNum();
	if (ParameterCount == 0)
		return;
	if (Parameter.size() < ParameterCount)
	{
		LOG_ERROR("Parameter size is not enough");
		return;
	}

	// Z Y X
	for (int i = 2; i >= 0; i--)
		if (static_cast<unsigned int>(DOF) & (1 << i))
			Location[i] = Parameter(-- ParameterCount);

	// Parameter should discard the used parameter form front of vector
	Parameter = Parameter.tail(Parameter.size() - ParameterNum()).eval();
}

VectorXd JointLocationParamter::GetParameter() const
{
	VectorXd Result(ParameterNum());
	int ParameterCount = ParameterNum();
	for (int i = 2; i >= 0; i--)
		if (static_cast<unsigned int>(DOF) & (1 << i))
			Result(-- ParameterCount) = Location[i];
	return Result;
}

JointRotationParamter::operator FVector&()
{
	return RotationEuler;
}
JointRotationParamter::operator FVector()
{
	return RotationEuler;
}

JointRotationParamter::operator Quaterniond()
{
	return MMath::QuaternionFromEulerXYZ(RotationEuler);
}

Matrix3d JointRotationParamter::MakeMatrix3d()
{
	double RotationX = RotationEuler[0];
	double RotationY = RotationEuler[1];
	double RotationZ = RotationEuler[2];
	Matrix3d RotationXM = Matrix3d::Identity();
	Matrix3d RotationYM = Matrix3d::Identity();
	Matrix3d RotationZM = Matrix3d::Identity();

	RotationXM(1,1) = std::cos(RotationX); RotationXM(1, 2) = -std::sin(RotationX);
	RotationXM(2, 1) = std::sin(RotationX); RotationXM(2, 2) = std::cos(RotationX);

	RotationYM(0, 0) = std::cos(RotationY); RotationYM(0, 2)  = std::sin(RotationY);
	RotationYM(2, 0) = -std::sin(RotationY); RotationYM(2, 2) = std::cos(RotationY);

	RotationZM(0, 0) = std::cos(RotationZ); RotationZM(0, 1) = -std::sin(RotationZ);
	RotationZM(1, 0) = std::sin(RotationZ); RotationZM(1, 1) = std::cos(RotationZ);

	return RotationZM * RotationYM *RotationXM;
}

Matrix4d JointRotationParamter::MakeMatrix4d()
{
	Matrix4d Result = Matrix4d::Identity();
	Result.block(0, 0, 2, 2) = MakeMatrix3d();
	return Result;
}

inline int JointRotationParamter::ParameterNum() const
{
	static std::map<EDOF3D, int> t{ { FreeNone, 0 }, { FreeX, 1 }, { FreeY, 1 }, { FreeZ, 1 },
		{ FreeXY, 2 }, { FreeXZ, 2 }, { FreeYZ, 2 }, { FreeXYZ, 3 } };
	return t[DOF];
}

inline void JointRotationParamter::ReadParameter(VectorXd& Parameter)
{
	int ParameterCount = ParameterNum();
	if (ParameterCount == 0) return;
	if (Parameter.size() < ParameterCount)
	{
		LOG_ERROR("Parameter size is not enough");
		return;
	}
	// Z Y X
	for (int i = 2; i >= 0; i--)
		if (static_cast<unsigned int>(DOF) & (1 << i))
			RotationEuler[i] = Parameter(-- ParameterCount);
	// Parameter should discard the used parameter form front of vector
	Parameter = Parameter.tail(Parameter.size() - ParameterNum()).eval();
}

VectorXd JointRotationParamter::GetParameter() const
{
	VectorXd Result(ParameterNum());
	int ParameterCount = ParameterNum();
	for (int i = 2; i >= 0; i--)
		if (static_cast<unsigned int>(DOF) & (1 << i))
			Result(-- ParameterCount) = RotationEuler[i];
	return Result;
}

JointParameter::JointParameter(EDOF3D InRoationDOF, EDOF3D InTranslationDOF)
	: LocationParam(InTranslationDOF), RotationParam(InRoationDOF)
{}

JointParameter::operator FTransform()
{
	ASSERTMSG(ParameterNum() > 0, "ParameterNum is 0");
	FTransform Result = FTransform::Identity();
	Result.SetRotation(RotationParam);
	Result.SetTranslation(LocationParam);
	return Result;
}

JointParameter::operator Matrix4d()
{
	ASSERTMSG(ParameterNum() > 0, "ParameterNum is 0");
	return LocationParam.MakeMatrix4d() * RotationParam.MakeMatrix4d();
}

int JointParameter::ParameterNum() const
{
	return LocationParam.ParameterNum() + RotationParam.ParameterNum();
}
void JointParameter::ReadParameter(VectorXd& Parameter)
{
	RotationParam.ReadParameter(Parameter);
	LocationParam.ReadParameter(Parameter);
}
FVector JointParameter::GetLocation() const
{
	return LocationParam.Location;
}
FVector JointParameter::GetRotationEuler() const
{
	return RotationParam.RotationEuler;
}

VectorXd JointParameter::GetParameter() const
{
	VectorXd Result(ParameterNum());
	int ParameterCount = 0;
	VectorXd RotationParameter = RotationParam.GetParameter();
	Result.segment(ParameterCount, RotationParameter.size()) = RotationParameter;
	ParameterCount += RotationParameter.size();
	VectorXd LocationParameter = LocationParam.GetParameter();
	Result.segment(ParameterCount, LocationParameter.size()) = LocationParameter;
	return Result;
}