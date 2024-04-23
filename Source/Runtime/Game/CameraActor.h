//
// Created by MarvelLi on 2024/2/5.
//

#pragma once
#include "Actor.h"
#include "Core/CoreMinimal.h"
#include "Components/CameraComponent.h"

MCLASS(CameraActor)
class CameraActor : public Actor
{
	REFLECTION_BODY(CameraActor)

protected:
	ObjectPtr<class CameraComponent> CameraComponent;

public:
	CameraActor()
	{
		CameraComponent = AddComponent<class CameraComponent>();
	}

	ObjectPtr<class CameraComponent> GetCameraComponent()
	{
		return CameraComponent;
	}
};
