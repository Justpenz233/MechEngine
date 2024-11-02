//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "TransformProxy.h"
#include "StaticMeshSceneProxy.h"
#include "Components/TransformComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{
TransformSceneProxy::TransformSceneProxy(GpuScene& InScene)
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
		auto TransformId = TransformIdMap[Component];
		transform_data& data = TransformDatas[TransformId];
		data.transformMatrix = ToLuisaMatrix(Component->GetTransformMatrix());
		data.scale = ToLuisaVector(Component->GetScale());
		data.rotationQuaternion = ToLuisaVector(Component->GetRotation().coeffs());
		if (TransformToInstanceId.count(TransformId))
		{
			accel.set_transform_on_update(TransformToInstanceId[TransformId], data.transformMatrix);
		}
	}
	stream << transform_buffer.subview(0, GetTransformCount()).copy_from(TransformDatas.data());
	DirtyTransforms.clear();
}

uint TransformSceneProxy::AddTransform(TransformComponent* InTransform)
{
	if (!InTransform)
	{
		LOG_WARNING("Trying to add a null transform to the scene.");
		return 0;
	}
	if (TransformIdMap.count(InTransform))
		return TransformIdMap[InTransform];
	uint NewId = Id++;
	TransformIdMap[InTransform] = NewId;
	DirtyTransforms.insert(InTransform);
	return NewId;
}
void TransformSceneProxy::BindTransform(uint InstanceID, uint TransformID)
{
	if (TransformID >= GetTransformCount())
	{
		LOG_ERROR("Trying to bind a transform with id: {} that does not exist in the scene.", TransformID);
		return;
	}
	if (accel.size() <= InstanceID)
	{
		LOG_ERROR("Trying to bind a transform to an instance with id: {} that does not exist in the scene.", InstanceID);
		return;
	}
	TransformToInstanceId[TransformID] = InstanceID;
	accel.set_transform_on_update(InstanceID, TransformDatas[TransformID].transformMatrix);
}

bool TransformSceneProxy::IsExist(TransformComponent* InTransform) const
{
	return TransformIdMap.count(InTransform);
}

bool TransformSceneProxy::IsExist(const uint TransformID) const
{
	return std::ranges::any_of(TransformIdMap, [TransformID](const auto& Pair) { return Pair.second == TransformID; });
}

void TransformSceneProxy::UpdateTransform(TransformComponent* InTransform)
{
	if(TransformIdMap.count(InTransform))
		DirtyTransforms.insert(InTransform);
}

}