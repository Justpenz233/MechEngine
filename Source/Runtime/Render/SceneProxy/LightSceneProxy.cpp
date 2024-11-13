//
// Created by MarvelLi on 2024/4/5.
//

#include "LightSceneProxy.h"
#include "ShapeSceneProxy.h"
#include "Components/AreaLightComponent.h"
#include "Components/ConstLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/light/lights.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{
LightSceneProxy::LightSceneProxy(GpuScene& InScene) noexcept
	: SceneProxy(InScene)
{
	LightDatas.resize(light_max_number);
	std::tie(light_buffer, bindless_id) = Scene.RegisterBindlessBuffer<light_data>(light_max_number);

	point_light_tag = light_virtual_call.create<point_light>(Scene);
	rectangle_light_tag = light_virtual_call.create<rectangle_light>(Scene);
	const_light_tag = light_virtual_call.create<const_light>(Scene);
}

uint LightSceneProxy::GetLightTypeTag(LightComponent* InLight) const
{
	if (InLight->IsA<PointLightComponent>())
		return point_light_tag;
	else if (InLight->IsA<ConstLightComponent>())
		return const_light_tag;
	else if (InLight->IsA<AreaLightComponent>())
		return rectangle_light_tag;
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
	LightData.intensity = ToLuisaVector(InLight->GetIntensity());
	LightData.light_color = ToLuisaVector(InLight->GetLightColor());
	if (auto Ptr = Cast<PointLightComponent>(InLight))
	{
		LightData.light_type = point_light_tag;
		LightData.size = {Ptr->GetRadius(), Ptr->GetRadius()};
	}
	else if (Cast<ConstLightComponent>(InLight))
	{
		LightData.light_type = const_light_tag;
	}
	else if (auto Ptr = Cast<AreaLightComponent>(InLight))
	{
		LightData.light_type = rectangle_light_tag;
		LightData.size = {
			static_cast<float>(Ptr->GetSize().x()),
			static_cast<float>(Ptr->GetSize().y())
		};
	}
	return LightData;
}
} // namespace MechEngine::Rendering