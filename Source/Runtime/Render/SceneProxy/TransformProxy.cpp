//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "TransformProxy.h"
#include "StaticMeshSceneProxy.h"
#include "Components/TransformComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
TransformSceneProxy::TransformSceneProxy(RayTracingScene& InScene)
	: SceneProxy(InScene)
{
	TransformDatas.resize(transform_matrix_buffer_size);
	std::tie(transform_buffer, bindless_id) = Scene.RegisterBindlessBuffer<transform_data>(transform_matrix_buffer_size);
}

void TransformSceneProxy::UploadDirtyData(Stream& stream)
{
	if (DirtyTransforms.empty())
		return;

	for (TransformComponent* Component : DirtyTransforms)
	{
		auto ComponentId = TransformIndexMap[Component];
		transform_data& data = TransformDatas[ComponentId];
		data.transformMatrix = ToLuisaMatrix(Component->GetTransformMatrix());
		data.scale = ToLuisaVector(Component->GetScale());
		data.rotationQuaternion = ToLuisaVector(Component->GetRotation().coeffs());
		if(auto InstanceId = Scene.GetStaticMeshProxy()->TransformIdToMeshId(ComponentId); InstanceId != ~0u)
		{
			accel.set_transform_on_update(InstanceId, data.transformMatrix);
		}
	}
	stream << transform_buffer.subview(0, GetTransformCount()).copy_from(TransformDatas.data());
	DirtyTransforms.clear();
}

uint TransformSceneProxy::AddTransform(TransformComponent* InTransform)
{
	if(!InTransform)
	{
		LOG_WARNING("Trying to add a null transform to the scene.");
		return 0;
	}
	if(TransformIndexMap.count(InTransform))
		return TransformIndexMap[InTransform];
	uint NewId = Id++;
	TransformIndexMap[InTransform] = NewId;
	DirtyTransforms.insert(InTransform);
	return NewId;
}

bool TransformSceneProxy::IsExist(TransformComponent* InTransform) const
{
	return TransformIndexMap.count(InTransform);
}

bool TransformSceneProxy::IsExist(const uint TransformID) const
{
	return std::ranges::any_of(TransformIndexMap, [TransformID](const auto& Pair) { return Pair.second == TransformID; });
}

void TransformSceneProxy::UpdateTransform(TransformComponent* InTransform)
{
	if(TransformIndexMap.count(InTransform))
		DirtyTransforms.insert(InTransform);
	else
		LOG_ERROR("Trying to update a transform from actor: {} that does not exist in the scene.", InTransform->GetOwnerName());
}

}