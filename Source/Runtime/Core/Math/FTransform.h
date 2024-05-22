#pragma once
#include "CoreMinimal.h"
#include "Reflection/reflection/reflection.h"

MSTRUCT(FTransform)
struct ENGINE_API FTransform
{
	REFLECTION_STRUCT_BODY(FTransform)
protected:
	MPROPERTY()
	FVector Translation = FVector::Zero();

	MPROPERTY()
	FQuat Rotation = FQuat::Identity();

	MPROPERTY()
	FVector Scale = FVector::Ones();

public:
	FTransform(
		const Vector3d& InTranslation = Vector3d::Zero(), const Quaterniond& InRotation = Quaterniond::Identity(), const Vector3d& InScale = Vector3d::Ones());
	~FTransform(){};

	static FTransform Identity();

	Matrix4d GetTranslationMatrix() const;
	Matrix4d GetRotationMatrix() const;
	Matrix4d GetScaleMatrix() const;
	Matrix4d GetMatrix() const;

	FORCEINLINE const FVector& GetTranslation() const { return Translation; }
	FORCEINLINE const FVector& GetLocation() const { return Translation; }
	FORCEINLINE const FQuat&   GetRotation() const { return Rotation; }
	FVector		   GetRotationEuler() const;
	FORCEINLINE const FVector& GetScale() const { return Scale; }

	FORCEINLINE void AddTranslationGlobal(const FVector& InTranslation) { Translation += InTranslation; }
	FORCEINLINE void AddTranslationLocal(const FVector& InTranslation) { Translation += Rotation * InTranslation; }
	FORCEINLINE void AddRotationGlobal(const FQuat& InRotation) { Rotation = InRotation * Rotation; }
	FORCEINLINE void AddRotationLocal(const FQuat& InRotation) { Rotation = Rotation * InRotation; }
	FORCEINLINE void AddScale(const FVector& InScale) { Scale += InScale; }
	FORCEINLINE void AddScale(double InScale) { Scale *= InScale; }

	/// \brief Random a transform, location in [0,1] , scale is 1.
	static FTransform Random();

	FORCEINLINE void SetTranslation(const Vector3d& InTranslation) { Translation = InTranslation; }
	FORCEINLINE void SetRotation(const Quaterniond& InRotation) { Rotation = InRotation; }
	FORCEINLINE void SetScale(const Vector3d& InScale) { Scale = InScale; }
	FTransform LerpTo(const FTransform& Other, double Alpha) const;

	FVector		ToLocalSpace(const FVector& WolrdLocation);
	FTransform	Inverse() const;
	FTransform& operator=(const FTransform& Other);
	FTransform& operator=(const Eigen::Affine3d& Other);

	bool operator==(const FTransform& Other) const;

	bool operator!=(const FTransform& Other) const;

	explicit operator Eigen::Affine3d() const;

	// When composing, Right first then Left.
	FTransform operator*(const FTransform& Other) const;
	FVector	   operator*(const FVector& Other) const;

	static FTransform Lerp(const FTransform&A, const FTransform& B, double Alpha);
};

template <>
struct fmt::formatter<FTransform> : fmt::formatter<std::string>
{
	auto format(const FTransform& a, format_context& ctx) const {
		std::string Context = fmt::format("\nLocation : ({}, {}, {})\nRotation: ({}, {}, {}) \nScale: ({}, {}, {})",
			a.GetTranslation().x(), a.GetTranslation().y(), a.GetTranslation().z(),
			a.GetRotationEuler().x(), a.GetRotationEuler().y(), a.GetRotationEuler().z(),
			a.GetScale().x(), a.GetScale().y(), a.GetScale().z()
			);
		return fmt::formatter<std::string>::format(Context, ctx);
	}
};


template <class T>
Eigen::Matrix<T, 4, 4> FTranslationMatrix(const Eigen::Matrix<T, 3, 1>& Translation)
{
	Eigen::Matrix<T, 4, 4> Result = Eigen::Matrix<T, 4, 4>::Identity();
	Result.block(0, 3, 3, 1) = Translation;
	return Result;
}

template <class T>
Eigen::Matrix<T, 4, 4> FScaleMatrix(const Eigen::Matrix<T, 3, 1>& Scale)
{
	Eigen::Matrix<T, 4, 4> Result = Eigen::Matrix<T, 4, 4>::Identity();
	Result.block(0, 0, 3, 3) = Scale.asDiagonal();
	return Result;
}

template <class T>
Eigen::Matrix<T, 4, 4> FRotationMatrix(const Eigen::Quaternion<T>& Rotation)
{
	Eigen::Matrix<T, 4, 4> Result = Eigen::Matrix<T, 4, 4>::Identity();
	Result.block(0, 0, 3, 3) = Rotation.toRotationMatrix();
	return Result;
}