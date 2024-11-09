//
// Created by MarvelLi on 2024/9/24.
//

#include "ShapeSceneProxy.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{

ShapeSceneProxy::ShapeSceneProxy(GpuScene& InScene)
	: SceneProxy(InScene)
{
	InstanceShapes.resize(InScene.MaxInstanceNum);
	std::tie(instance_shape, buffer_bindless_id) = InScene.RegisterBindlessBuffer<shape>(InScene.MaxInstanceNum);
}

void ShapeSceneProxy::UploadDirtyData(Stream& stream)
{
	if (!bDirty) return;
	bDirty = false;

	stream << instance_shape.copy_from(InstanceShapes.data());
}

void ShapeSceneProxy::SetInstanceLightID(uint InstanceID, uint LightID)
{
	InstanceShapes[InstanceID].light_id = LightID;
	bDirty = true;
}

void ShapeSceneProxy::SetInstanceMeshID(uint InstanceID, uint MeshID)
{
	InstanceShapes[InstanceID].mesh_id = MeshID;
	bDirty = true;
}

uint ShapeSceneProxy::RegisterInstance() const
{
	accel.emplace_back(0);
	return accel.size() - 1;
}

void ShapeSceneProxy::RemoveInstance(uint InstanceID) const
{
	if (InstanceID >= accel.size())
	{
		LOG_ERROR("Trying to remove an instance that does not exist. ID {} >= {}", InstanceID, accel.size());
		return;
	}
	accel.set_visibility_on_update(InstanceID, false);
}

void ShapeSceneProxy::SetInstanceVisibility(uint InstanceID, bool bVisible)
{
	if (InstanceID >= accel.size())
	{
		LOG_ERROR("Trying to remove an instance that does not exist. ID {} >= {}", InstanceID, accel.size());
		return;
	}
	accel.set_visibility_on_update(InstanceID, bVisible);
}

} // namespace MechEngine::Rendering