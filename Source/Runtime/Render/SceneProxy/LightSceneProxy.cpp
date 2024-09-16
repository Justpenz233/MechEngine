//
// Created by MarvelLi on 2024/4/5.
//

#include "LightSceneProxy.h"

#include "Components/ConstPointLightComponent.h"
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
	std::tie(light_buffer, bindless_id) = Scene.RegisterBindlessBuffer<light_data>(light_max_number);

	point_light_tag = light_virtual_call.create<point_light>();
	const_light_tag = light_virtual_call.create<const_point_light>();
}

uint LightSceneProxy::GetLightTypeTag(LightComponent* InLight) const
{
	if (InLight->IsA<PointLightComponent>())
		return point_light_tag;
	else if (InLight->IsA<ConstPointLightComponent>())
		return const_light_tag;
	else
		return ~0u;
}

void LightSceneProxy::AddLight(LightComponent* InLight, uint InTransformID)
{
	if(InLight != nullptr)
	{
		ASSERTMSG(LightIndexMap.count(InLight) == 0, "Light already exists in the batch");
		if(GetLightTypeTag(InLight) != ~0u)
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
	else
	{
		LOG_ERROR("Trying to add nullptr light to scene");
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
		else if (Cast<ConstPointLightComponent>(Light))
		{
			LightData.light_type = const_light_tag;
		}
		else
		{
			LOG_ERROR( "Light type not supported" );
		}
	}
	DirtyLights.clear();

	stream << light_buffer.copy_from(LightDatas.data());
}
}