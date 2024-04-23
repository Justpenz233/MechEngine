#pragma once
#include <iostream>

#include "RenderingComponent.h"
#include "Math/FTransform.h"
#include "Components/ActorComponent.h"
#include "Core/CoreMinimal.h"
#include "Math/Math.h"


// By default, the camera is at the origin, looking up the z-axis, with the positive x-axis pointing to.
// In World space, the world X is Z(depth), the world Y is X(Left), and the world Z is Y(Height).
// In Camera space, the camera +X is Right, the camera +Y is Up, and the camera +Z is Forward.
// Same as UE
// @wiki: https://smiling-calcium-8e1.notion.site/Space-8bea9b9e55d4435da0ecaaca6389d6bc?pvs=4
MCLASS(CameraComponent)
class CameraComponent : public RenderingComponent
{
	REFLECTION_BODY(CameraComponent)

protected:
	// Field of view in degrees
	MPROPERTY()
	float FovH = 45.;

	MPROPERTY()
	float Zoom = 1.;

public:
    bool IsDirty = false;

    CameraComponent() = default;
	~CameraComponent() = default;

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;
	virtual void BeginPlay() override;
    virtual void TickComponent(double DeltaTime) override;

	inline void MarkDirty() { IsDirty = true; }
	inline void ClearDirty() { IsDirty = false; }

	FORCEINLINE float GetFovH() const { return FovH; }
	FORCEINLINE float GetFovV() const { return 2.f * atanf(tanf(DegToRad(FovH) / 2.) / GetAspectRatio()); }
	FORCEINLINE float GetZoom() const { return Zoom; }
	FORCEINLINE float GetAspectRatio() const { return World->GetViewport()->GetAspectRatio(); }


	/**
	 * Get view matrix, transform world space to camera space
	 * @return View matrix
	 */
	FORCEINLINE FMatrix4 GetViewMatrix() const;

	/**
	 * Get projection matrix, transform camera space to clip space
	 * @return Projection matrix
	 */
	FORCEINLINE FMatrix4 GetProjectionMatrix() const;

	/**
	 * Project world space to clip space
	 * @param Pos Position in world space
	 * @return Clip space position
	 */
	FORCEINLINE FVector4 ProjectClipSpace(const FVector& Pos) const;

	/**
	 * Project world space to clip space, with normalized device coordinates
	 * @see https://smiling-calcium-8e1.notion.site/Projection-transform-739348862c1248bf978af8121375d7b8?pvs=4
	 * @param Pos Position in world space
	 * @return Clip space position
	 */
	FORCEINLINE FVector Project(const FVector& Pos) const;

	/**
	 * Project clip space to world space
	 * @see https://smiling-calcium-8e1.notion.site/Projection-transform-739348862c1248bf978af8121375d7b8?pvs=4
	 * @param NormalClipPos Normalized position in clip space
	 * @return Position in world space
	 */
	// FORCEINLINE FVector UnProject(const Vector4d& ClipPos) const;
	FORCEINLINE FVector UnProject(const FVector& NormalClipPos) const;


protected:
	virtual void UploadRenderingData() override;
};

FORCEINLINE FMatrix4 CameraComponent::GetViewMatrix() const
{
	Matrix4d ViewMatrix;
	ViewMatrix
		<<  0, 1, 0, 0,
			0, 0, 1, 0,
			1, 0, 0, 0,
			0, 0, 0, 1;
	return ViewMatrix * GetOwner()->GetFTransform().Inverse().GetMatrix();
}

FORCEINLINE FMatrix4 CameraComponent::GetProjectionMatrix() const
{
	FMatrix4 Proj = FMatrix4::Zero();
	float f = 1.0f / std::tan(DegToRad(FovH) * 0.5f);
	Proj(0, 0) = f;
	Proj(1, 1) = f * GetAspectRatio();
	Proj(2, 2) = 1.f;
	Proj(2, 3) = -1.0f;
	Proj(3, 2) = 1.0f;
	return Proj;
}

FORCEINLINE FVector4 CameraComponent::ProjectClipSpace(const FVector& Pos) const
{
	return GetProjectionMatrix() * (GetViewMatrix() * MakeVector4d(Pos, 1.));
}


FORCEINLINE FVector CameraComponent::Project(const FVector& Pos) const
{
	auto ScreenPos = ProjectClipSpace(Pos);
	return ScreenPos.w() == 0.? ScreenPos.head(3) : (ScreenPos.head(3) / ScreenPos.w()).eval();
}

FORCEINLINE FVector CameraComponent::UnProject(const FVector& ClipPos) const
{
	auto Pos = (GetProjectionMatrix() * GetViewMatrix()).inverse() * FVector4{ClipPos.x(), ClipPos.y(), ClipPos.z(), 1.};
	return Pos.head(3) / Pos.w();
}