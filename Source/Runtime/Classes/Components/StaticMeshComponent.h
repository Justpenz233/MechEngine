//
// Created by Mayn on 8/25/2023.
//

#pragma once
#include "Core/CoreMinimal.h"
#include "SceneComponent.h"
#include "Mesh/StaticMesh.h"

class Material;

enum StaticMeshDirtyTag: unsigned int
{
	DIRTY_NONE       = 0x0000,
	DIRTY_RENDERDATA = 0x0001,   // Render Data dirty, need upload, include material and geometry
	DIRTY_REMESH       = 0x0010, // Property dirty, need remesh geometry. Alough static mesh should not be remeshed, but it is possible to remesh it. FOR NOW
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

inline StaticMeshDirtyTag& operator ^= (StaticMeshDirtyTag& ATag, StaticMeshDirtyTag& BTag)
{
	unsigned int Result = ATag ^ BTag;
	ATag = static_cast<StaticMeshDirtyTag>(Result);
	return ATag;
}


MCLASS(StaticMeshComponent)
class ENGINE_API StaticMeshComponent : public SceneComponent
{
	REFLECTION_BODY(StaticMeshComponent)
public:
	StaticMeshComponent();
	~StaticMeshComponent() override;

	MPROPERTY()
	ObjectPtr<StaticMesh> MeshData = nullptr;

	ObjectPtr<StaticMesh> CollisionMesh = nullptr;

	FORCEINLINE void               MarkAsDirty(StaticMeshDirtyTag InTag);
	FORCEINLINE StaticMeshDirtyTag GetDirtyTag() const;
	FORCEINLINE void               ClearDirty();
	virtual bool                   IsDirty();

	virtual void BeginPlay() override;
	virtual void TickComponent(double DeltaTime) override;
	virtual void EndPlay() override;
	virtual void Destroy() override;

	virtual void OnSelected() override;

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	void SetVisible(bool InVisible);
	FORCEINLINE bool IsVisible() const;

	virtual void UploadRenderingData() override;

	/**
	 * Interface to remesh this geometry
	 * Will be called when marked as DIRTY_REMESH
	 */
	virtual void Remesh();

	ObjectPtr<StaticMesh> GetMeshData() const;
	ObjectPtr<StaticMesh> GetStaticMesh() const;
	ObjectPtr<StaticMesh> GetCollisionMesh() const;

	void SetMeshData(ObjectPtr<StaticMesh> InMeshData);
	void SetCollisionMeshData(const ObjectPtr<StaticMesh>& InCollisionMesh);

	void SmoothMesh(int Iteration = 5, bool UseUniform = false);

	bool FillHoles();

	/**
	 * Set the color of the mesh, this is a quick interface to set the base color of the current mesh material
	 * @param InColor color to set
	 */
	void SetColor(const FColor& InColor) const;
	/**
	 * Set the material of the mesh, this is a quick interface to set the material of the current mesh
	 * @param InMaterial material to set
	 */
	void SetMaterial(const ObjectPtr<Material>& InMaterial);

	FORCEINLINE uint GetMeshID() const { return MeshID; }
	FORCEINLINE uint GetInstanceID() const { return InstanceID; }

protected:
	StaticMeshDirtyTag Dirty = DIRTY_RENDERDATA;

	// Instance id for the mesh that this component holds
	uint InstanceID = ~0u;

	uint MeshID = ~0u;
};

FORCEINLINE bool StaticMeshComponent::IsVisible() const
{
	return bVisible;
}

FORCEINLINE void StaticMeshComponent::MarkAsDirty(StaticMeshDirtyTag InTag)
{
	Dirty |= InTag;
}

FORCEINLINE StaticMeshDirtyTag StaticMeshComponent::GetDirtyTag() const
{
	return Dirty;
}

FORCEINLINE void StaticMeshComponent::ClearDirty()
{
	Dirty = DIRTY_NONE;
}