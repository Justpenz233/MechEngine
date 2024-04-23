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

	virtual void BeginPlay() override;

private:

    MPROPERTY()
    float TranslationSpeed = 0.02f; // Translation speed in world space unit

    void MouseLeftDragRotation(FVector2 StartPos, FVector2 Delta);

	void MouseRightDragTranslation(FVector2 StartPos, FVector2 Delta);

    void MouseWheelZoom(FVector2 Delta);

    void KeyPressedEvent(int Key);

    CameraComponent* Component;
};

FORCEINLINE CameraComponent* CameraActor::GetCameraComponent() const
{
    return Component;
}