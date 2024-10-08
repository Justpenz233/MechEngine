//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "Game/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/AreaLightComponent.h"

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

MCLASS(AreaLightActor)
class AreaLightActor : public Actor
{
	REFLECTION_BODY(AreaLightActor)
public:
	AreaLightActor()
	{
		Component = AddComponent<AreaLightComponent>().get();
	}

	FORCEINLINE AreaLightComponent* GetLightComponent() const
	{
		return Component;
	}
protected:
	AreaLightComponent* Component;
};