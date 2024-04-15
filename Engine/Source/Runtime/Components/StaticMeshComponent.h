//
// Created by Mayn on 8/25/2023.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "RenderingComponent.h"
#include "Mesh/StaticMesh.h"

class Material;

enum StaticMeshDirtyTag: unsigned int
{
	DIRTY_NONE       = 0x0000,
	DIRTY_RENDERDATA = 0x0001,   // Render Data dirty, need upload
	DIRTY_REMESH       = 0x0010, // Property dirty, need remesh
	DIRTY_MATERIAL       = 0x0100, // Property dirty, need remesh
	DIRTY_ALL        = 0xFFFF
};

inline StaticMeshDirtyTag& operator |= (StaticMeshDirtyTag& ATag, StaticMeshDirtyTag& BTag)
{
	unsigned int Result = ATag | BTag;
	ATag = static_cast<StaticMeshDirtyTag>(Result);
	return ATag;
}

inline StaticMeshDirtyTag& operator &= (StaticMeshDirtyTag& ATag, StaticMeshDirtyTag& BTag)
{
	unsigned int Result = ATag & BTag;
	ATag = static_cast<StaticMeshDirtyTag>(Result);
	return ATag;
}

MCLASS(StaticMeshComponent)
class StaticMeshComponent : public RenderingComponent
{
	REFLECTION_BODY(StaticMeshComponent)
public:
	StaticMeshComponent();
	~StaticMeshComponent() override;

	inline static FColor DefalutColor = Vector3d(0.8, 0.8, 0.8);   //  1: Light Gray

	MPROPERTY()
	ObjectPtr<StaticMesh> MeshData;

	ObjectPtr<StaticMesh> CollisionMesh;

	FORCEINLINE virtual void MarkAsDirty(StaticMeshDirtyTag InTag);
	virtual bool IsDirty();

	virtual void BeginPlay() override;
	virtual void TickComponent(double DeltaTime) override;
	virtual void EndPlay() override;

	virtual void OnSelected() override;

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	void SetVisible(bool InVisible);
	FORCEINLINE bool IsVisible() const;

	void SetMeshNormal(const MeshNormalOption& Option);

	// Upload raw data to renderer
	// Will be called when marked as DIRTY_RENDERDATA
	virtual void UploadRenderingData() override;

	// Interface to remesh this geometry
	// Will be called when marked as DIRTY_REMESH
	virtual void Remesh();

	ObjectPtr<StaticMesh> GetMeshData();
	ObjectPtr<StaticMesh> GetCollisionMesh();

	void SetMeshData(ObjectPtr<StaticMesh> InMeshData);
	void SetCollisionMeshData(ObjectPtr<StaticMesh> InCollisionMesh);

	static FColor GetNextColor();
	// Set color and override color of mesh data
	FORCEINLINE void SetColor(FColor NewColor);

	void SmoothMesh(int Iteration = 5, bool UseUniform = false);

	bool FillHoles();

protected:
	MPROPERTY()
	FColor Color = Vector3d(0.8, 0.8, 0.8);

	StaticMeshDirtyTag Dirty = DIRTY_ALL;
};

FORCEINLINE bool StaticMeshComponent::IsVisible() const
{
	return bVisible;
}

FORCEINLINE void StaticMeshComponent::MarkAsDirty(StaticMeshDirtyTag InTag)
{
	Dirty |= InTag;
}

FORCEINLINE void StaticMeshComponent::SetColor(FColor NewColor)
{
	Color = NewColor;
	MarkAsDirty(StaticMeshDirtyTag::DIRTY_RENDERDATA);
}
