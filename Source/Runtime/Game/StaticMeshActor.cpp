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

StaticMeshComponent* StaticMeshActor::GetStaticMeshComponent()
{
	return MeshComponent;
}
