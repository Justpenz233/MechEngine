//
// Created by MarvelLi on 2024/5/6.
//

#pragma once
#include "LightComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"

/**
 * A point light component that emits light from a single point equally in all directions.
 * With no attenuation, the light would not attenuate with distance.
 * Not physically correct, useful for debugging geometry.
 */
MCLASS(ConstPointLightComponent)
class ConstPointLightComponent : public LightComponent
{
	REFLECTION_BODY(ConstPointLightComponent)

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