//
// Created by Mayn on 2024/10/6.
//

#include "AreaLightComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
void AreaLightComponent::UploadRenderingData()
{
	if(bDirty)
	{
		Remesh();
		LightComponent::UploadRenderingData();
		if (MeshId == ~0u)
			MeshId = GetWorld()->GetScene()->GetStaticMeshProxy()->AddStaticMesh(MeshData.get());
		else
			GetWorld()->GetScene()->GetStaticMeshProxy()->UpdateStaticMeshGeometry(MeshId, MeshData.get());

		if(MeshId != ~0u && InstanceId != ~0u)
		{
			GetWorld()->GetScene()->GetShapeProxy()->SetInstanceMeshID(InstanceId, MeshId);
			GetWorld()->GetScene()->GetStaticMeshProxy()->BindInstance(MeshId, InstanceId);
		}
	}
	bDirty = false;
}

void AreaLightComponent::Remesh()
{
	MeshData = BasicShapesLibrary::GeneratePlane(Size);
	MeshData->RotateEuler(FVector{M_PI, 0., 0.});
}

void AreaLightComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	LightComponent::PostEdit(Field);
	if (Field == NAME(Size))
	{
		MarkAsDirty();
		UploadRenderingData();
	}
}

