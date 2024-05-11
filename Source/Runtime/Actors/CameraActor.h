#pragma once
#include "Components/CameraComponent.h"
#include "Core/CoreMinimal.h"
#include "Game/Actor.h"

MCLASS(CameraActor)
class CameraActor : public Actor
{
    REFLECTION_BODY(CameraActor)
public:
    CameraActor();

    FORCEINLINE CameraComponent* GetCameraComponent() const;

    FORCEINLINE FVector GetFocusCenter() const;

    /**
     * Set the view matrix of the camera
     * @tparam T matrix scalar type
     * @param InView New view matrix
     */
    template<typename T>
    FORCEINLINE void SetView(const Eigen::MatrixX4<T>& InView);

    /**
     * Set the camera to look at the target position
     * @param Target the target position
     */
    void LookAt(const FVector &Target);

    /**
     * Set the camera to look at the current target position
     */
    void LookAt();

	/**
	 * Blend the camera to the target transform
	 * @param TargetTransform the target transform
	 * @param Duration the duration of the blending
	 */
	void BlendeTo(const FTransform& TargetTransform, double Duration = 1.);

	/**
	 * Blend the camera to the target location
	 * @param TargetLocation the target location
	 * @param Duration the duration of the blending
	 */
	void BlendeTo(const FVector& TargetLocation, double Duration = 1.);

	virtual void BeginPlay() override;

private:
    MPROPERTY()
    float RotationSpeed = 1.f;

    MPROPERTY()
    float TranslationSpeed = 1.f;

	MPROPERTY()
	bool bFoucsOnTarget = true;

    MPROPERTY()
    FVector FocusCenter = {0., 0., 0.};

    void MouseLeftDragRotation(FVector2 StartPos, FVector2 Delta);

	void MouseRightDragTranslation(FVector2 StartPos, FVector2 Delta);

    void MouseWheelZoom(FVector2 Delta);

    CameraComponent* Component;

	FVector2 LastPos = {0.f, 0.f};
	FVector2 LastDelta = {0.f, 0.f};
	FQuat LastRotation;
	double DistranceToTarget;
};

FORCEINLINE CameraComponent* CameraActor::GetCameraComponent() const
{
    return Component;
}

FORCEINLINE FVector CameraActor::GetFocusCenter() const
{
    return FocusCenter;
}

template<typename T>
FORCEINLINE void CameraActor::SetView(const Eigen::MatrixX4<T>& InView)
{

}