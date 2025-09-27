//
// Created by Mayn on 8/25/2023.
//

#include "StaticMeshComponent.h"
#include "Game/Actor.h"
#include "Game/World.h"
#include "Materials/Material.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

StaticMeshComponent::StaticMeshComponent()
{
	MeshData = NewObject<StaticMesh>();
	CollisionMesh = NewObject<StaticMesh>();
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::TickComponent(double DeltaTime)
{
	if (IsDirty())
	{
		if(Dirty & DIRTY_REMESH)
			Remesh();
		if(Dirty & DIRTY_RENDERDATA)
			UploadRenderingData();
		Dirty = DIRTY_NONE;
	}
}

void StaticMeshComponent::BeginPlay()
{
	ActorComponent::BeginPlay();
	if (IsDirty())
	{
		if(Dirty & DIRTY_REMESH)
			Remesh();
		if(Dirty)
			UploadRenderingData();
		Dirty = DIRTY_NONE;
	}
}

void StaticMeshComponent::EndPlay()
{
	ActorComponent::EndPlay();
}

void StaticMeshComponent::Destroy()
{
	SceneComponent::Destroy();
	if(GetWorld() != nullptr)
	{
		if(auto ShapeProxy = GetScene()->GetShapeProxy()) // Check nullptr prevent a system shut down
		{
			ShapeProxy->RemoveInstance(InstanceID);
		}
		if (auto StaticMeshProxy = GetScene()->GetStaticMeshProxy())
		{
			StaticMeshProxy->RemoveStaticMesh(MeshID);
		}
	}
}

void StaticMeshComponent::OnSelected()
{
	ActorComponent::OnSelected();
}

void StaticMeshComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	SceneComponent::PostEdit(Field);
	String FieldName = Field.getFieldName();
	if (FieldName == NAME(bVisible))
	{
		World->GetScene()->GetShapeProxy()->SetInstanceVisibility(InstanceID, bVisible);
	}
}

void StaticMeshComponent::SetVisible(bool InVisible)
{
	bVisible = InVisible;
	if(InstanceID != ~0u)
		World->GetScene()->GetShapeProxy()->SetInstanceVisibility(InstanceID, bVisible);
}

void StaticMeshComponent::UploadRenderingData()
{
	SceneComponent::UploadRenderingData();
	if(MeshData != nullptr && !MeshData->IsEmpty())
	{
		ASSERTMSG(MeshData->GetMaterial() != nullptr, "Material should not be null!");
		ASSERTMSG(MeshData->CheckNormalValid(), "Geometry is modified, but not called GeometryChanged.");
		if(MeshID == ~0u)
			MeshID = World->GetScene()->GetStaticMeshProxy()->AddStaticMesh(MeshData.get());
		else
			World->GetScene()->GetStaticMeshProxy()->UpdateStaticMeshGeometry(MeshID, MeshData.get());

		if (InstanceID == ~0u)
			InstanceID = GetScene()->GetShapeProxy()->RegisterInstance();

		if (InstanceID != ~0u)
		{
			GetScene()->GetShapeProxy()->SetInstanceMeshID(InstanceID, MeshID);
			GetWorld()->GetScene()->GetTransformProxy()->BindTransform(InstanceID, TransformId);
			GetScene()->GetShapeProxy()->SetInstanceMeshID(InstanceID, MeshID);
			GetScene()->GetStaticMeshProxy()->BindInstance(MeshID, InstanceID);
		}
		World->GetScene()->GetShapeProxy()->SetInstanceVisibility(InstanceID, bVisible);
	}
}

void StaticMeshComponent::Remesh()
{
	MarkAsDirty(DIRTY_RENDERDATA);
}

ObjectPtr<StaticMesh> StaticMeshComponent::GetMeshData() const
{
	return MeshData;
}

ObjectPtr<StaticMesh> StaticMeshComponent::GetStaticMesh() const
{
	return MeshData;
}

ObjectPtr<StaticMesh> StaticMeshComponent::GetCollisionMesh() const
{
	return CollisionMesh;
}

void StaticMeshComponent::SetMeshData(ObjectPtr<StaticMesh> InMeshData)
{
	MeshData = std::move(InMeshData);
	MarkAsDirty(DIRTY_RENDERDATA);
}

void StaticMeshComponent::SetCollisionMeshData(const ObjectPtr<StaticMesh>& InCollisionMesh)
{
	CollisionMesh = InCollisionMesh;
	MarkAsDirty(DIRTY_RENDERDATA);
}

void StaticMeshComponent::SmoothMesh(int Iteration, bool UseUniform)
{
	MeshData->SmoothMesh(Iteration, UseUniform);
	MarkAsDirty(DIRTY_RENDERDATA);
}

bool StaticMeshComponent::FillHoles()
{
	bool Flag = MeshData->FillHoles();
	if (Flag)
		MarkAsDirty(DIRTY_RENDERDATA);
	return Flag;
}
void StaticMeshComponent::SetColor(const FColor& InColor) const
{
	if (MeshData && MeshData->GetMaterial() != nullptr)
		MeshData->GetMaterial()->SetBaseColor(InColor);
}

void StaticMeshComponent::SetMaterial(const ObjectPtr<Material>& InMaterial)
{
	if (MeshData && InMaterial != nullptr)
	{
		MeshData->SetMaterial(InMaterial);
		MarkAsDirty(DIRTY_RENDERDATA);
	}
};

bool StaticMeshComponent::IsDirty()
{
	return Dirty != StaticMeshDirtyTag::DIRTY_NONE;
}
