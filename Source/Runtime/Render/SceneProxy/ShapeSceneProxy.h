//
// Created by MarvelLi on 2024/9/24.
//

#pragma once
#include "SceneProxy.h"
#include "Render/Core/shape.h"

namespace MechEngine::Rendering
{
using namespace luisa::compute;

class ENGINE_API ShapeSceneProxy : public SceneProxy
{
public:
	ShapeSceneProxy(GpuScene& InScene);

	void UploadDirtyData(Stream& stream) override;

	/**
	 * Set an instance as a light
	 * @param InstanceID instance id
	 * @param LightID light id
	 */
	void SetInstanceLightID(uint InstanceID, uint LightID);

	/**
	 * Set an instance as a mesh
	 * @param InstanceID instance id
	 * @param MeshID mesh id
	 */
	void SetInstanceMeshID(uint InstanceID, uint MeshID);

	/**
	 * Register a new instance, need to set shape information later
	 * @return the instance id
	 */
	[[nodiscard]] uint RegisterInstance() const;

	/**
	 * Remove an instance
	 * @param InstanceID instance id
	 */
	void RemoveInstance(uint InstanceID) const;

	/**
	 * Set the visibility of the instance
	 * @param InstanceID instance id
	 * @param bVisible visibility
	 */
	void SetInstanceVisibility(uint InstanceID, bool bVisible);

public:
	Var<shape> get_instance_shape(const UInt& instance_id) const
	{
		return bindelss_buffer<shape>(buffer_bindless_id)->read(instance_id);
	}

protected:
	bool bDirty = true;
	uint buffer_bindless_id;
	BufferView<shape> instance_shape;
	vector<shape> InstanceShapes;
};
}
