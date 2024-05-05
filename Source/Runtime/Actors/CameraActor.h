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

    template<typename T>
    FORCEINLINE void SetView(const Eigen::MatrixX4<T>& InView);

	virtual void BeginPlay() override;

private:

    MPROPERTY()
    float RotationSpeed = 1.f;

    MPROPERTY()
    float TranslationSpeed = 1.f;

    MPROPERTY()
    FVector FocusCenter = {0., 0., 0.};

    void MouseLeftDragRotation(FVector2 StartPos, FVector2 Delta);

	void MouseRightDragTranslation(FVector2 StartPos, FVector2 Delta);

    void MouseWheelZoom(FVector2 Delta);

    CameraComponent* Component;
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