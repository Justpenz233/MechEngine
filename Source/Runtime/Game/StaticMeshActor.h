//
// Created by Li Siqi, CIH-I on 2023/6/15.
//

#pragma once

#include "Core/CoreMinimal.h"
#include "Game/Actor.h"
#include "Components/StaticMeshComponent.h"

MCLASS(StaticMeshActor)
class ENGINE_API StaticMeshActor : public Actor
{
public:
	REFLECTION_BODY(StaticMeshActor)

	StaticMeshActor();

	explicit StaticMeshActor(ObjectPtr<StaticMesh> InMeshData);

	StaticMeshComponent* GetStaticMeshComponent();

    virtual void SetVisible(bool bVisible);

	Math::FBox GetBoundingBox() const;

	/**
	 * Fast interface to set the material of the controlled mesh
	 * @param InMaterial The material to set
	 */
	void SetMaterial(const ObjectPtr<Material>& InMaterial);

	/**
	 * Get the world mesh of the actor. Which is to transform the mesh data by the actor's transform
	 * @return The world mesh
	 */
	ObjectPtr<StaticMesh> GetWorldMesh() const;

protected:
	StaticMeshComponent* MeshComponent;
};
