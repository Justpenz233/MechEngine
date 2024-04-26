//
// Created by MarvelLi on 2024/4/5.
//

#include "LightSceneProxy.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/light/point_light.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
LightSceneProxy::LightSceneProxy(RayTracingScene& InScene) noexcept
	: SceneProxy(InScene)
{
	LightDatas.resize(light_max_number);
	light_buffer = Scene.RegisterBuffer<light_data>(light_max_number);

	point_light_tag = light_virtual_call.create<point_light>();
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
		auto& LightData = LightDatas[LightIndexMap[Light].first];
		LightData.intensity = Light->GetIntensity();
		LightData.light_color = ToLuisaVector(Light->GetLightColor());
		LightData.transform_id = LightIndexMap[Light].second;
		if(auto Ptr = Cast<PointLightComponent>(Light))
		{
			LightData.light_type = point_light_tag;
			LightData.radius = Ptr->GetRadius();
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