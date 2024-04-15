//
// Created by MarvelLi on 2024/4/5.
//

#include "LightSceneProxy.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
LightSceneProxy::LightSceneProxy(RayTracingScene& InScene) noexcept
	: SceneProxy(InScene)
{
	LightDatas.resize(light_max_number);
	light_buffer = Scene.RegisterBuffer<lightData>(light_max_number);
}

void LightSceneProxy::AddLight(LightComponent* InLight, uint InTransformID)
{
	ASSERTMSG(LightIndexMap.count(InLight) == 0, "Light already exists in the batch");
	if(auto Ptr = Cast<PointLightComponent>(InLight))
	{
		DirtyLights.insert(InLight);
		LightIndexMap[InLight] = {id, InTransformID};
		id += 1;
	}
	else
	{
		LOG_ERROR( "Light type not supported" );
	}
}

void LightSceneProxy::UpdateLight(LightComponent* InLight)
{
	DirtyLights.insert(InLight);
}

void LightSceneProxy::UploadDirtyData(Stream& stream)
{
	if( DirtyLights.empty()) { return; }
	for(auto& Light : DirtyLights)
	{
		if(auto Ptr = Cast<PointLightComponent>(Light))
		{
			auto& LightData = LightDatas[LightIndexMap[Light].first];
			LightData.intensity = Ptr->GetIntensity();
			LightData.linear_color = ToLuisaVector(Ptr->GetLightColor());
			LightData.transform_id = LightIndexMap[Light].second;
			LightData.samples_per_pixel = Ptr->GetSamplesPerPixel();
		}
		else
		{
			LOG_ERROR( "Light type not supported" );
		}
	}
	DirtyLights.clear();

	stream << light_buffer.subview(0, id).copy_from(LightDatas.data());
}
}