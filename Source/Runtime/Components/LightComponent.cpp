//
// Created by MarvelLi on 2024/4/6.
//

#pragma once
#include "LightComponent.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

void LightComponent::BeginPlay()
{
	UploadRenderingData();
}

void LightComponent::UploadRenderingData()
{

	if (InstanceId == ~0u)
		InstanceId = GetScene()->GetShapeProxy()->RegisterInstance();
	if (LightId == ~0u)
		LightId = GetScene()->GetLightProxy()->AddLight(this, InstanceId);
	else
		GetScene()->GetLightProxy()->UpdateLight(this, LightId, InstanceId);
	GetScene()->GetShapeProxy()->SetInstanceLightID(InstanceId, LightId);

	auto TransformId = GetScene()->GetTransformProxy()->AddTransform(GetOwner()->GetTransformComponent());
	GetScene()->GetTransformProxy()->BindTransform(InstanceId, TransformId);
}

float LightComponent::GetLightSourceTotalArea() const
{
	ASSERTMSG(false, "LightComponent::GetLightSourceTotalArea() is not implemented");
	return 0.0f;
}
