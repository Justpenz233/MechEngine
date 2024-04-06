#pragma once
#include "Components/CameraComponent.h"
#include "Core/CoreMinimal.h"
#include "Game/Actor.h"

class CameraActor : public Actor
{
public:
    CameraActor();
    ~CameraActor();

    CameraComponent* GetCameraComponent() const;

	virtual void BeginPlay() override;

private:

    void MouseLeftDragRotation(Vector2f StartPos, Vector2f Delta);

	void MouseRightDragTranslation(Vector2f StartPos, Vector2f Delta);

    CameraComponent* Camera;
};
