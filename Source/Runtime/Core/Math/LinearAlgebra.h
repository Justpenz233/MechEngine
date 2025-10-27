//
// Created by Mavel Li on 17/9/23.
//

#pragma once
#include "CoreMinimal.h"
namespace MMath
{

	ENGINE_API FVector2 RotateVector2D(const FVector2& Vec, double AngleRad);

	//Get Euler angle of rotation order XYZ
	ENGINE_API FVector EulerFromQuaternionXYZ(const Eigen::Quaterniond& Rotation);

	//follow right hand rule, in radian
	ENGINE_API double GetRotationAngle(const Quaterniond& Rotation);
	//follow right hand rule
	ENGINE_API FVector GetRotationAxis(const Quaterniond& Rotation);

	ENGINE_API Matrix3d RotationMatrixFromEulerXYZ(const FVector& Rotation);
	ENGINE_API Matrix4d RotationMatrix4FromEulerXYZ(const FVector& Rotation);

	ENGINE_API FVector VectorMultiply(const FVector& A, const FVector& B);
	//Euler angle in {x, y, z}
	ENGINE_API FQuat QuaternionFromEulerXYZ(const FVector& Rotation);

	ENGINE_API Matrix4d MakeTranslationMatrix(const FVector& Translation);

	ENGINE_API Matrix4d MakeScaleMatrix(const FVector& Scale);

	ENGINE_API Matrix4d MakeTransformMatrix(const FVector& Translation = FVector::Zero(), const Quaterniond& Rotation = Quaterniond::Identity(), const FVector& Scale = FVector::Ones());
}

namespace LinearAlgbera
{
	// Solve Ax = B, or min (Ax - B)^2
	ENGINE_API MatrixXd LinearEquationSolver(const MatrixXd& A, const MatrixXd& B);

	/**
	 * Calculate the LookAt matrix
	 * @param Eye the eye position
	 * @param Target the target position
	 * @param Up the up vector
	 * @return 3X3 rotation matrix
	 */
	ENGINE_API FMatrix LookAtMatrix(const FVector& Eye, const FVector& Target, FVector Up = FVector{0, 0, 1});
}