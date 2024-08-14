//
// Created by Li Siqi, CIH-I on 2023/6/15.
//

#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

StaticMeshActor::StaticMeshActor()
{
	MeshComponent = AddComponent<StaticMeshComponent>().get();
}
StaticMeshActor::StaticMeshActor(ObjectPtr<StaticMesh> InMeshData)
{
	MeshComponent = AddComponent<StaticMeshComponent>().get();
	MeshComponent->SetMeshData(std::move(InMeshData));
}

void StaticMeshActor::SetVisible(bool bVisible) {
    MeshComponent->SetVisible(bVisible);
}

Math::FBox StaticMeshActor::GetBoundingBox() const
{
	return MeshComponent->GetMeshData()->GetBoundingBox().Translate(GetTranslation());
}

void StaticMeshActor::SetMaterial(const ObjectPtr<Material>& InMaterial)
{
	ASSERT(MeshComponent != nullptr);
	if (MeshComponent)
		MeshComponent->SetMaterial(InMaterial);
}

ObjectPtr<StaticMesh> StaticMeshActor::GetWorldMesh() const
{
	auto Result = NewObject<StaticMesh>(*MeshComponent->GetMeshData());
	Result->TransformMesh(GetFTransform());
	return Result;
}

StaticMeshComponent* StaticMeshActor::GetStaticMeshComponent()
{
	return MeshComponent;
}
