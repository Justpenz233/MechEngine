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
	transform_buffer = Scene.RegisterBuffer<transformData>(transform_matrix_buffer_size);
}

void TransformSceneProxy::UploadDirtyData(Stream& stream)
{
	if (DirtyTransforms.empty())
	{
		return;
	}

	for (TransformComponent* Component : DirtyTransforms)
	{
		auto ComponentId = TransformIndexMap[Component];
		transformData& data = TransformDatas[ComponentId];
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
	ASSERTMSG(TransformIndexMap.count(InTransform) == 0, "TransformComponent already exist in scene!");
	uint NewId = Id++;
	TransformIndexMap[InTransform] = NewId;
	DirtyTransforms.insert(InTransform);
	return NewId;
}

void TransformSceneProxy::UpdateTransform(TransformComponent* InTransform)
{
	ASSERTMSG(TransformIndexMap.count(InTransform), "TransformComponent not exist in scene!");
	DirtyTransforms.insert(InTransform);
}

}