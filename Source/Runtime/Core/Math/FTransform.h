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

	[[nodiscard]] Matrix4d GetTranslationMatrix() const;
	[[nodiscard]] Matrix4d GetRotationMatrix() const;
	[[nodiscard]] Matrix4d GetScaleMatrix() const;
	[[nodiscard]] Matrix4d GetMatrix() const;

	[[nodiscard]] FORCEINLINE const FVector& GetTranslation() const { return Translation; }
	[[nodiscard]] FORCEINLINE const FVector& GetLocation() const { return Translation; }
	[[nodiscard]] FORCEINLINE const FQuat&   GetRotation() const { return Rotation; }
	[[nodiscard]] FVector GetRotationEuler() const;
	[[nodiscard]] FORCEINLINE const FVector& GetScale() const { return Scale; }

	FORCEINLINE FTransform& AddTranslationGlobal(const FVector& InTranslation) { Translation += InTranslation; return *this; }
	FORCEINLINE FTransform& AddTranslationLocal(const FVector& InTranslation) { Translation += Rotation * InTranslation; return *this; }
	FORCEINLINE FTransform& AddRotationGlobal(const FQuat& InRotation) { Rotation = InRotation * Rotation; return *this; }
	FORCEINLINE FTransform& AddRotationLocal(const FQuat& InRotation) { Rotation = Rotation * InRotation; return *this; }
	FORCEINLINE FTransform& AddScale(const FVector& InScale) { Scale += InScale; return *this;}
	FORCEINLINE FTransform& AddScale(double InScale) { Scale *= InScale; return *this;}

	/// \brief Random a transform, location in [0,1] , scale is 1.
	static FTransform Random();

	FORCEINLINE FTransform& SetTranslation(const Vector3d& InTranslation) { Translation = InTranslation; return *this; }
	FORCEINLINE FTransform& SetRotation(const Quaterniond& InRotation) { Rotation = InRotation; return *this; }
	FORCEINLINE FTransform& SetScale(const Vector3d& InScale) { Scale = InScale; return *this; }
	[[nodiscard]] FTransform LerpTo(const FTransform& Other, double Alpha) const;

	/**
	 * Get local location relative to this transform
	 * Inverse itself can't concatenate with VQS format(since VQS always transform from S->Q->T, where inverse happens from T(-1)->Q(-1)->S(-1))
	 * @param WolrdLocation Location in world space
	 * @return Location in local space
	 */
	[[nodiscard]] FVector		ToLocalSpace(const FVector& WolrdLocation) const;
	[[nodiscard]] FTransform	Inverse() const;
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