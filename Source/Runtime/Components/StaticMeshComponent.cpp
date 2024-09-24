//
// Created by Mayn on 8/25/2023.
//

#include "StaticMeshComponent.h"
#include "Game/Actor.h"
#include "Game/World.h"
#include "Materials/Material.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

StaticMeshComponent::StaticMeshComponent()
{
	MeshData = NewObject<StaticMesh>();
	CollisionMesh = NewObject<StaticMesh>();
}

StaticMeshComponent::~StaticMeshComponent()
{
	if(GetWorld() != nullptr)
	{
		if(auto SceneProxy = GetScene()->GetStaticMeshProxy()) // Check nullptr prevent a system shut down
		{
			SceneProxy->RemoveStaticMesh(MeshID);
		}
	}
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
		if(Dirty & DIRTY_RENDERDATA)
			UploadRenderingData();
		Dirty = DIRTY_NONE;
	}
}

void StaticMeshComponent::EndPlay()
{
	ActorComponent::EndPlay();
}

void StaticMeshComponent::OnSelected()
{
	ActorComponent::OnSelected();
}

void StaticMeshComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	RenderingComponent::PostEdit(Field);
	String FieldName = Field.getFieldName();
	if (FieldName == NAME(bVisible))
	{
		World->GetScene()->GetStaticMeshProxy()->SetInstanceVisibility(InstanceID, bVisible);
	}
}

void StaticMeshComponent::SetVisible(bool InVisible)
{
	bVisible = InVisible;
	World->GetScene()->GetStaticMeshProxy()->SetInstanceVisibility(InstanceID, bVisible);
}

void StaticMeshComponent::UploadRenderingData()
{
	if(MeshData != nullptr && !MeshData->IsEmpty())
	{
		ASSERTMSG(MeshData->GetMaterial() != nullptr, "Material should not be null!");
		ASSERTMSG(MeshData->CheckNormalValid(), "Geometry is modified, but not called GeometryChanged.");
		if(MeshID == ~0u)
			MeshID = World->GetScene()->GetStaticMeshProxy()->AddStaticMesh(MeshData.get());
		else
			World->GetScene()->GetStaticMeshProxy()->UpdateStaticMeshGeometry(MeshID, MeshData.get());

		if (InstanceID == ~0u)
		{
			auto TransformId = World->GetScene()->GetTransformProxy()->AddTransform(GetOwner()->GetTransformComponent());
			InstanceID = GetScene()->GetStaticMeshProxy()->AddInstance(MeshID, TransformId);
		}
		else
			GetScene()->GetStaticMeshProxy()->UpdateInstance(InstanceID, MeshID);
	}
}
void StaticMeshComponent::Remesh()
{
	MarkAsDirty(DIRTY_RENDERDATA);
}

ObjectPtr<StaticMesh> StaticMeshComponent::GetMeshData()
{
	return MeshData;
}
ObjectPtr<StaticMesh> StaticMeshComponent::GetStaticMesh()
{
	return MeshData;
}

ObjectPtr<StaticMesh> StaticMeshComponent::GetCollisionMesh()
{
	return CollisionMesh;
}

void StaticMeshComponent::SetMeshData(ObjectPtr<StaticMesh> InMeshData)
{
	MeshData = std::move(InMeshData);
	MarkAsDirty(DIRTY_RENDERDATA);
}

void StaticMeshComponent::SetCollisionMeshData(ObjectPtr<StaticMesh> InCollisionMesh)
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

void StaticMeshComponent::SetMaterial(ObjectPtr<Material> InMaterial)
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
