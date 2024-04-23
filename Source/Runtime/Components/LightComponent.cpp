//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "LightComponent.h"

void LightComponent::BeginPlay()
{
	GetScene()->AddLight(this, GetOwner()->GetTransformComponent());
}

void LightComponent::UploadRenderingData()
{
	GetScene()->UpdateLight(this);
}
