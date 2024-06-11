//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "Game/Actor.h"
#include "Components/PointLightComponent.h"

MCLASS(PointLightActor)
class PointLightActor : public Actor
{
	REFLECTION_BODY(PointLightActor)
public:
	PointLightActor()
	{
		Component = AddComponent<PointLightComponent>().get();
	}

	FORCEINLINE PointLightComponent* GetPointLightComponent() const
	{
		return Component;
	}

protected:
	PointLightComponent* Component;
};