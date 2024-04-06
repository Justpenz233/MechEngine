//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "LightComponent.h"
#include "Components/ActorComponent.h"

MCLASS(PointLightComponent)
class PointLightComponent : public LightComponent
{
	REFLECTION_BODY(PointLightComponent)
public:
	PointLightComponent() = default;
	~PointLightComponent() = default;
};