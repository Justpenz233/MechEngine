//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "LightComponent.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

void LightComponent::BeginPlay()
{
	auto TransformId = GetScene()->GetTransformProxy()->AddTransform(GetOwner()->GetTransformComponent());
	GetScene()->GetLightProxy()->AddLight(this, TransformId);
}

void LightComponent::UploadRenderingData()
{
	GetScene()->GetLightProxy()->UpdateLight(this);
}
