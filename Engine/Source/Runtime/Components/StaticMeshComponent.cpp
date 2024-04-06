//
// Created by Mayn on 8/25/2023.
//

#include "StaticMeshComponent.h"
#include "Game/Actor.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"

StaticMeshComponent::StaticMeshComponent()
{
	MeshData = NewObject<StaticMesh>();
	CollisionMesh = NewObject<StaticMesh>();
}

StaticMeshComponent::~StaticMeshComponent()
= default;

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
	// Add the mesh to the scene
	World->GetScene()->AddStaticMesh(this, GetOwner()->GetTransformComponent());
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
	World->GetScene()->EraseMesh(this);
}

void StaticMeshComponent::OnSelected()
{
	ActorComponent::OnSelected();
}

void StaticMeshComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	ActorComponent::PostEdit(Field);
	String FieldName = Field.getFieldName();
	if (FieldName == NAME(Color) || FieldName == NAME(bVisible))
		UploadRenderingData();
}

void StaticMeshComponent::SetVisible(bool InVisible)
{
	if(InVisible != bVisible)
		MarkAsDirty(DIRTY_RENDERDATA);
	bVisible = InVisible;
}

void StaticMeshComponent::SetMeshNormal(const MeshNormalOption& Option)
{
	MeshData->SetupNormal(Option);
	MarkAsDirty(DIRTY_RENDERDATA);
}

void StaticMeshComponent::UploadRenderingData()
{
	if(MeshData != nullptr && !MeshData->IsEmpty())
	{
		if (!MeshData->CheckNormalValid())
			MeshData->CalcNormal();

		MeshData->SetColor(Color);

		World->GetScene()->UpdateStaticMesh(this);
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

Vector3d StaticMeshComponent::GetNextColor()
{
	static int Index = 0;
	FColor ColorTable[16] = {
        Vector3d(0.9, 0.6, 0.4),   //  1: Orange
        Vector3d(0.6, 0.6, 0.9),   //  2: Light Blue
        Vector3d(0.4, 0.9, 0.4),   //  3: Green
        Vector3d(0.4, 0.4, 0.9),   //  4: Blue
        Vector3d(0.8, 0.8, 0.8),   //  5: Light Gray

        Vector3d(0.9, 0.5, 0.5),   //  6: Red
        Vector3d(0.9, 0.9, 0.5),   //  7: Yellow
        Vector3d(0.7, 0.3, 0.9),   //  8: Purple
        Vector3d(0.4, 0.9, 0.9),   //  9: Cyan
        Vector3d(0.9, 0.3, 0.6),   // 10: Pink
        Vector3d(0.6, 0.4, 0.3),   // 11: Brown
        Vector3d(0.9, 0.6, 0.5),   // 12: LightSalmon

        Vector3d(0.5, 0.2, 0.5),   // 13: Dark Purple
        Vector3d(0.4, 0.8, 0.7),   // 14: Dark Cyan
        Vector3d(0.3, 0.4, 0.7),   // 15: Dark Blue
        Vector3d(0.6, 0.6, 0.3),   // 16: Dark Yellow
		};
	Index %= 16;
	return ColorTable[Index ++];
}

void StaticMeshComponent::SmoothMesh(int Iteration, bool UseUniform)
{
	MeshData->SmoothMesh(Iteration, UseUniform);
	MarkAsDirty(DIRTY_RENDERDATA);
}

bool StaticMeshComponent::FillHoles()
{
	bool Flag = MeshData->FillHoles();
	if(Flag) MarkAsDirty(DIRTY_RENDERDATA);
	return Flag;
};

bool StaticMeshComponent::IsDirty()
{
	return Dirty != StaticMeshDirtyTag::DIRTY_NONE;
}
