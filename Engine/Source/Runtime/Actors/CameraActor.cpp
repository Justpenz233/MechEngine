#include "CameraActor.h"

#include <iostream>

#include "Components/CameraComponent.h"
#include "Game/Actor.h"

CameraActor::CameraActor() : Actor()
{
    Camera = AddComponent<CameraComponent>().get();
}

CameraActor::~CameraActor()
{
    
}

CameraComponent* CameraActor::GetCameraComponent() const
{
    return Camera;
}
void CameraActor::BeginPlay()
{
	Actor::BeginPlay();
	World->GetViewport()->MouseLeftButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseLeftDragRotation);
	World->GetViewport()->MouseRightButtonDragEvent.AddObjectFunction(this, &CameraActor::MouseRightDragTranslation);
}

void CameraActor::MouseLeftDragRotation(Vector2f StartPos, Vector2f Delta)
{

}

void CameraActor::MouseRightDragTranslation(Vector2f StartPos, Vector2f Delta)
{
	static Vector2f LastPos = {0.f, 0.f};
	static Vector2f LastDelta = {0.f, 0.f};
	if(StartPos == LastPos)
		AddTranslationLocal({0, -(Delta.x() - LastDelta.x()) * 0.01, (Delta.y() - LastDelta.y()) * 0.01});
	LastPos = StartPos;
	LastDelta = Delta;
}