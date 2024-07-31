//
// Created by Mavel Li on 17/9/23.
//
// Edit by Rick Zhang on July 25, 2024

#include "LinearAlgebra.h"
#include <iostream>


double NormalizeAngle(double angle) {

	angle = std::fmod(angle, 360.0);
	if (angle < 0) {
		angle += 2*M_PI;
	}

	return angle;
}


namespace MMath
{
FVector EulerFromQuaternionXYZ(const Quaterniond& Rotation)
{
	FVector Euler = Rotation.toRotationMatrix().eulerAngles(2, 1, 0);
	std::swap(Euler[0], Euler[2]);


	Euler[0] = NormalizeAngle(Euler[0]);
	Euler[1] = NormalizeAngle(Euler[1]);
	Euler[2] = NormalizeAngle(Euler[2]);

	return Euler;
}

double GetRotationAngle(const Quaterniond& Rotation)
{
	Eigen::Matrix3d	  rotationMatrix = Rotation.toRotationMatrix();
	Eigen::AngleAxisd angleAxis(rotationMatrix);
	return angleAxis.angle();
}

FVector GetRotationAxis(const Quaterniond& Rotation)
{
	Eigen::Matrix3d	  rotationMatrix = Rotation.toRotationMatrix();
	Eigen::AngleAxisd angleAxis(rotationMatrix);
	return angleAxis.axis();
}

Quaterniond QuaternionFromEulerXYZ(const FVector& Rotation)
{



	return Eigen::AngleAxisd(Rotation[2], FVector::UnitZ()) * Eigen::AngleAxisd(Rotation[1], FVector::UnitY()) * Eigen::AngleAxisd(Rotation[0], FVector::UnitX());
}

Matrix3d RotationMatrixFromEulerXYZ(const FVector& Rotation)
{
	return QuaternionFromEulerXYZ(Rotation).toRotationMatrix();
}

Matrix4d RotationMatrix4FromEulerXYZ(const FVector& Rotation)
{
	Eigen::Affine3d Rotation90 = Affine3d::Identity();
	Rotation90.rotate(MMath::RotationMatrixFromEulerXYZ(Rotation));
	return Rotation90.matrix();
}

FVector VectorMultiply(const FVector& A, const FVector& B)
{
	return A.array() * B.array();
}

Matrix4d MakeTranslationMatrix(const FVector& Translation)
{
	Matrix4d Result = Matrix4d::Identity();
	for (int i = 0; i < 3; i++)
		Result(i, 3) = Translation[i];
	return Result;
}

Matrix4d MakeScaleMatrix(const FVector& Scale)
{
	Matrix4d Result = Matrix4d::Identity();
	for (int i = 0; i < 3; i++)
		Result(i, i) = Scale[i];
	return Result;
}

Matrix4d MakeTransformMatrix(const FVector& Translation, const Quaterniond& Rotation, const FVector& Scale)
{
	Eigen::Affine3d RotationMatrix = Affine3d::Identity();
	RotationMatrix.rotate(Rotation);
	return MakeTranslationMatrix(Translation) * RotationMatrix.matrix() * MakeScaleMatrix(Scale);
}
} // namespace MMath

MatrixXd LinearAlgbera::LinearEquationSolver(const MatrixXd& A, const MatrixXd& B)
{
	// Solve Ax = B, or min (Ax - B)^2
	// A is a n * m matrix, B is a n * k matrix, x is a m * k matrix
	// return x
	// if A is not a square matrix, use least square method to solve it
	// if A is a square matrix, use LU decomposition to solve it
	if (A.rows() != B.rows())
	{
		std::cout << "LinearEquationSolver: A.rows() != B.rows()" << std::endl;
		return MatrixXd::Zero(0, 0);
	}
	if (A.rows() == A.cols())
	{
		// A is a square matrix
		return A.lu().solve(B);
	}
	else
	{
		// A is not a square matrix
		return (A.transpose() * A).lu().solve(A.transpose() * B);
	}
}
FMatrix LinearAlgbera::LookAtMatrix(const FVector& Eye, const FVector& Target, FVector Up)
{
	FVector Forward = (Target - Eye).normalized();
	FVector Right = FVector{ 0, 1, 0 };
	// if forward is parallel to Z axis, then we need to rotate around X axis
	if (abs(Forward.dot(Up)) > 0.99)
	{
		Up = Forward.cross(Right).normalized();
		Right = Up.cross(Forward).normalized();
	}
	else
	{
		Right = Up.cross(Forward).normalized();
		Up = Forward.cross(Right).normalized();
	}
	FMatrix RotationMatrix = FMatrix::Identity();
	RotationMatrix.col(0) = Forward;
	RotationMatrix.col(1) = Right;
	RotationMatrix.col(2) = Up;
	return RotationMatrix;
}