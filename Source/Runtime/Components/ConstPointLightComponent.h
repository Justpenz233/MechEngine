//
// Created by MarvelLi on 2024/5/6.
//

#pragma once
#include "LightComponent.h"

/**
 * A point light component that emits light from a single point equally in all directions.
 * With no attenuation, the light would not attenuate with distance.
 * Not physically correct, useful for debugging geometry.
 */
MCLASS(ConstPointLightComponent)
class ConstPointLightComponent : public LightComponent
{
	REFLECTION_BODY(ConstPointLightComponent)
};