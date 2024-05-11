#include "FTransform.h"
#include "LinearAlgebra.h"
#include "Random.h"

FTransform::FTransform(const Vector3d& InTranslation, const Quaterniond& InRotation, const Vector3d& InScale)
{
	Translation = InTranslation;
	Rotation = InRotation;
	Scale = InScale;
}

FTransform FTransform::Identity()
{
	return FTransform();
}

Matrix4d FTransform::GetTranslationMatrix() const
{
	return MMath::MakeTranslationMatrix(Translation);
}

Matrix4d FTransform::GetRotationMatrix() const
{
	return Eigen::Affine3d(Rotation).matrix();
}

Matrix4d FTransform::GetScaleMatrix() const
{
	return MMath::MakeScaleMatrix(Scale);
}

Matrix4d FTransform::GetMatrix() const
{
	return GetTranslationMatrix() * GetRotationMatrix() * GetScaleMatrix();
}

FVector FTransform::GetRotationEuler() const
{
	return MMath::EulerFromQuaternionXYZ(Rotation);
}

FTransform FTransform::Random()
{
	FTransform Result = FTransform::Identity();
	Result.SetTranslation({ Random::Random(), Random::Random(), Random::Random() });
	Result.SetRotation(Random::RandomQuat());
	return Result;
}

FTransform FTransform::LerpTo(const FTransform& Other, double Alpha) const
{
	return FTransform::Lerp(*this, Other, Alpha);
}

FVector FTransform::ToLocalSpace(const FVector& WolrdLocation)
{
	Matrix4d InverseTransfrom = GetMatrix().inverse();
	Vector4d Location;
	Location << WolrdLocation, 1;
	Location = InverseTransfrom * Location;
	return { Location[0], Location[1], Location[2] };
}

FTransform FTransform::Inverse() const
{
	FTransform Result;
	Result.SetScale({ 1. / Scale[0], 1. / Scale[1], 1. / Scale[2] });
	Result.SetRotation(Rotation.inverse());
	Result.SetTranslation(Rotation.inverse() * (Scale.asDiagonal() * Translation) * -1.);
	return Result;
}

FTransform& FTransform::operator=(const FTransform& Other)
{
	Translation = Other.Translation;
	Rotation = Other.Rotation;
	Scale = Other.Scale;
	return *this;
}

FTransform& FTransform::operator=(const Eigen::Affine3d& Other)
{
	Translation = Other.translation();
	Rotation = Other.rotation();
	Scale = {Other.linear().col(0).norm(), Other.linear().col(1).norm(), Other.linear().col(2).norm()};
	return *this;
}

bool FTransform::operator==(const FTransform& Other) const
{
	return Translation.isApprox(Other.Translation) && Rotation.coeffs().isApprox(Other.Rotation.coeffs()) && Scale.isApprox(Other.Scale);
}

bool FTransform::operator!=(const FTransform& Other) const
{
	return !Translation.isApprox(Other.Translation) || !Rotation.coeffs().isApprox(Other.Rotation.coeffs()) || !Scale.isApprox(Other.Scale);
}

FTransform::operator Eigen::Transform<double, 3, 2>() const
{
	return Eigen::Affine3d(GetMatrix());
}

// Transform time Transform
FTransform FTransform::operator*(const FTransform& Other) const
{
	FTransform Result;
	Result.Rotation = Rotation * Other.Rotation;
	Result.Translation = Rotation * (Scale.asDiagonal() * Other.Translation) + Translation;
	Result.Scale = { Scale.x() * Other.Scale.x(), Scale.y() * Other.Scale.y(), Scale.z() * Other.Scale.z() };
	return Result;
}

FVector FTransform::operator*(const FVector& Other) const
{
	return FVector4(GetMatrix() * Other.homogeneous()).head(3);
}

FTransform FTransform::Lerp(const FTransform& A, const FTransform& B, double Alpha)
{
	FTransform Result;
	Result.Translation = A.Translation * (1. - Alpha) + B.Translation * Alpha;
	Result.Rotation = A.Rotation.slerp(Alpha, B.Rotation);
	Result.Scale = A.Scale * (1. - Alpha) + B.Scale * Alpha;
	return Result;
}

