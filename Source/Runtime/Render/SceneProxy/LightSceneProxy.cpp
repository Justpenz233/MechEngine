//
// Created by MarvelLi on 2024/4/5.
//

#include "LightSceneProxy.h"

#include "ShapeSceneProxy.h"
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

uint LightSceneProxy::AddLight(LightComponent* InLight, uint InstanceId)
{
	if (InLight != nullptr)
	{
		if (GetLightTypeTag(InLight) == ~0u)
		{
			LOG_ERROR("Light type not supported");
			return ~0u;
		}
		auto LightID = IdCounter++;
		LightDatas[LightID] = GetFlatLightData(InLight);
		LightDatas[LightID].instance_id = InstanceId;
		bDirty = true;
		return LightID;
	}
	LOG_ERROR("Trying to add nullptr light to scene");
	return ~0u;
}

void LightSceneProxy::UpdateLight(LightComponent* InLight, uint LightId, uint InstanceId)
{
	if(InLight == nullptr)
	{
		LOG_ERROR("Trying to update nullptr light");
		return;
	}
	if(GetLightTypeTag(InLight) == ~0u)
	{
		LOG_ERROR( "Light type not supported" );
		return;
	}
	LightDatas[LightId] = GetFlatLightData(InLight);
	LightDatas[LightId].instance_id = InstanceId;
	bDirty = true;
}


void LightSceneProxy::UploadDirtyData(Stream& stream)
{
	if (!bDirty) return;
	bDirty = false;
	stream << light_buffer.copy_from(LightDatas.data());
}

light_data LightSceneProxy::GetFlatLightData(LightComponent* InLight) const
{
	light_data LightData;
	LightData.intensity = InLight->GetIntensity();
	LightData.light_color = ToLuisaVector(InLight->GetLightColor());
	if (auto Ptr = Cast<PointLightComponent>(InLight))
	{
		LightData.light_type = point_light_tag;
		LightData.radius = Ptr->GetRadius();
	}
	else if (Cast<ConstPointLightComponent>(InLight))
	{
		LightData.light_type = const_light_tag;
	}
	return LightData;
}
} // namespace MechEngine::Rendering