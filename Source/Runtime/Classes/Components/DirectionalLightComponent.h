//
// Created by MarvelLi on 2025/9/26.
//

#pragma once
#include "LightComponent.h"

MCLASS()
class ENGINE_API DirectionalLightComponent : public LightComponent
{
	REFLECTION_BODY()
public:
	DirectionalLightComponent() = default;
	~DirectionalLightComponent() = default;
};
