//
// Created by MarvelLi on 2024/5/6.
//

#pragma once
#include "LightComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"

/**
 * A point that illuminates the scene uniformly in all directions without attenuation and visibility
 */
MCLASS(ConstLightComponent)
class ConstLightComponent : public LightComponent
{
	REFLECTION_BODY(ConstLightComponent)
public:
	ConstLightComponent() : LightComponent()
	{
		bVisible = false; // always invisible
	}
protected:
	virtual void UploadRenderingData() override
	{
		LightComponent::UploadRenderingData();
		if (MeshId == ~0u)
		{
			MeshData = BasicShapesLibrary::GeneratePlane(); //minimum shape
			MeshId = GetWorld()->GetScene()->GetStaticMeshProxy()->AddStaticMesh(MeshData.get());
		}
		else
			GetWorld()->GetScene()->GetStaticMeshProxy()->UpdateStaticMeshGeometry(MeshId, MeshData.get());

		if(MeshId != ~0u && InstanceId != ~0u)
		{
			GetWorld()->GetScene()->GetShapeProxy()->SetInstanceMeshID(InstanceId, MeshId);
			GetWorld()->GetScene()->GetShapeProxy()->SetInstanceVisibility(InstanceId, false);
			GetWorld()->GetScene()->GetStaticMeshProxy()->BindInstance(MeshId, InstanceId);
		}
		bDirty = false;
	}
	ObjectPtr<StaticMesh> MeshData;
	uint MeshId = ~0u;
};