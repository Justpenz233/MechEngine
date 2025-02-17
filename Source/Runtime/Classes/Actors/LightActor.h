//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "Game/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/AreaLightComponent.h"
#include "Components/ConstLightComponent.h"

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


MCLASS(ConstLightActor)
class ConstLightActor : public Actor
{
	REFLECTION_BODY(ConstLightActor)
public:
	ConstLightActor()
	{
		Component = AddComponent<ConstLightComponent>().get();
	}

	FORCEINLINE ConstLightComponent* GetLightComponent() const
	{
		return Component;
	}
protected:
	ConstLightComponent* Component;
};
