//
// Created by Mavel Li on 17/9/23.
//

#pragma once
#include "CoreMinimal.h"
namespace MMath
{
	//Get Euler angle of rotation order XYZ
	FVector EulerFromQuaternionXYZ(const Eigen::Quaterniond& Rotation);

	//follow right hand rule, in radian
	double GetRotationAngle(const Quaterniond& Rotation);
	//follow right hand rule
	FVector GetRotationAxis(const Quaterniond& Rotation);

	Matrix3d RotationMatrixFromEulerXYZ(const FVector& Rotation); 
	Matrix4d RotationMatrix4FromEulerXYZ(const FVector& Rotation); 

	FVector VectorMultiply(const FVector& A, const FVector& B);
	//Euler angle in {x, y, z}
	FQuat QuaternionFromEulerXYZ(const FVector& Rotation);

	Matrix4d MakeTranslationMatrix(const FVector& Translation);

	Matrix4d MakeScaleMatrix(const FVector& Scale);

	Matrix4d MakeTransformMatrix(const FVector& Translation = FVector::Zero(), const Quaterniond& Rotation = Quaterniond::Identity(), const FVector& Scale = FVector::Ones());
}

namespace LinearAlgbera
{
	// Solve Ax = B, or min (Ax - B)^2
	MatrixXd LinearEquationSolver(const MatrixXd& A, const MatrixXd& B);
}