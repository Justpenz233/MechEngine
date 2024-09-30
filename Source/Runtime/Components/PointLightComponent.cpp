//
// Created by MarvelLi on 2024/9/23.
//
#include "PointLightComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

void PointLightComponent::BeginPlay()
{
	Remesh();
	UploadRenderingData();
}

void PointLightComponent::UploadRenderingData()
{
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

void PointLightComponent::Remesh()
{
	if (Radius < 1e-4)
		MeshData = BasicShapesLibrary::GenerateSphere(0.01, 32);
	else
		MeshData = BasicShapesLibrary::GenerateSphere(Radius, 32);
}

void PointLightComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	LightComponent::PostEdit(Field);
	if (Field == NAME(Radius))
	{
		Remesh();
		UploadRenderingData();
	}
}