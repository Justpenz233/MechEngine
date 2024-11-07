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
	for (int i = 0; i < Id;i ++)
	{
		transform_data& data = TransformDatas[i];
		data.last_transform_matrix = data.transform_matrix;
	}

	for (TransformComponent* Component : NewTransforms)
	{
		auto TransformId = TransformIdMap[Component];
		transform_data& data = TransformDatas[TransformId];

		data.transform_matrix = ToLuisaMatrix(Component->GetTransformMatrix());
		data.last_transform_matrix = data.transform_matrix; // New transform, last frame's transform matrix the same as the current frame

		data.inverse_transform_matrix = ToLuisaMatrix(Component->GetTransformMatrix().inverse().eval());
		data.scale = ToLuisaVector(Component->GetScale());
		data.rotation_quaternion = ToLuisaVector(Component->GetRotation().coeffs());
		if (TransformToInstanceId.count(TransformId))
		{
			accel.set_transform_on_update(TransformToInstanceId[TransformId], data.transform_matrix);
		}
	}

	for (TransformComponent* Component : DirtyTransforms)
	{
		auto TransformId = TransformIdMap[Component];
		transform_data& data = TransformDatas[TransformId];
		data.last_transform_matrix = data.transform_matrix;
		data.transform_matrix = ToLuisaMatrix(Component->GetTransformMatrix());
		data.inverse_transform_matrix = ToLuisaMatrix(Component->GetTransformMatrix().inverse().eval());
		data.scale = ToLuisaVector(Component->GetScale());
		data.rotation_quaternion = ToLuisaVector(Component->GetRotation().coeffs());
		if (TransformToInstanceId.count(TransformId))
		{
			accel.set_transform_on_update(TransformToInstanceId[TransformId], data.transform_matrix);
		}
	}
	stream << transform_buffer.subview(0, GetTransformCount()).copy_from(TransformDatas.data());
	DirtyTransforms.clear();
	NewTransforms.clear();
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
	NewTransforms.insert(InTransform);
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
	accel.set_transform_on_update(InstanceID, TransformDatas[TransformID].transform_matrix);
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