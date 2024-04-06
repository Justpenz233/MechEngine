//
// Created by Li Siqi, CIH-I on 2023/6/15.
//

#pragma once

#include "Core/CoreMinimal.h"
#include "Game/Actor.h"
#include "Components/StaticMeshComponent.h"

MCLASS(StaticMeshActor)
class StaticMeshActor : public Actor
{
protected:
	ObjectPtr<StaticMeshComponent> MeshComponent;
public:
	REFLECTION_BODY(StaticMeshActor)

	StaticMeshActor();

	explicit StaticMeshActor(ObjectPtr<StaticMesh> InMeshData);

	StaticMeshComponent* GetStaticMeshComponent();

    virtual void SetVisible(bool bVisible);

	Math::FBox GetBoundingBox() const;
};
