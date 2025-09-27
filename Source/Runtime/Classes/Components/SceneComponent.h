//
// Created by MarvelLi on 2024/3/29.
//

#pragma once

#include "ActorComponent.h"
#include "Game/World.h"

namespace MechEngine::Rendering
{
	class GpuSceneInterface;
}

/**
* SceneComponent is a base class for components that are used to render or visualize things in the world.
* This component should have methods to upload data to the GPU by interacting with GPU Scene.
* Each RenderingComponent should have a corresponding TransformComponent to store the transform data.
*/
MCLASS(SceneComponent)
class ENGINE_API SceneComponent : public ActorComponent
{
	REFLECTION_BODY(SceneComponent)

public:
	SceneComponent() = default;

	FORCEINLINE Rendering::GpuSceneInterface* GetScene() const
	{
		return World == nullptr ? nullptr :  World->GetScene();
	}

	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	FORCEINLINE virtual void TickComponent(double DeltaTime) override;

	virtual void BeginPlay() override
	{
		ActorComponent::BeginPlay();
		UploadRenderingData();
	}

	virtual void SetOwner(Actor* Owner) override;
	virtual void SetVisible(bool bInVisible) { bVisible = bInVisible; MarkAsDirty(); }

	FORCEINLINE FTransform& GetTransform() { return LocalTransform; }
	FORCEINLINE FTransform GetWorldTransform()
	{
		if (GetOwner() != nullptr)
			return LocalTransform * GetOwner()->GetFTransform();
		return LocalTransform;
	}
	FORCEINLINE FMatrix4 GetWorldMatrix(){return GetWorldTransform().GetMatrix();}
	FORCEINLINE FTransform GetLocalTransform(){return LocalTransform;}
	FORCEINLINE FMatrix4 GetLocalMatrix(){ return GetLocalTransform().GetMatrix(); }


protected:
	MPROPERTY()
	bool bVisible = true;

	MPROPERTY()
	FTransform LocalTransform = FTransform::Identity();

	virtual void UploadRenderingData();

	uint TransformId = ~0;
	bool bDirty = false;

	FORCEINLINE void MarkAsDirty();

};

FORCEINLINE void SceneComponent::TickComponent(double DeltaTime)
{
	ActorComponent::TickComponent(DeltaTime);
	if (bDirty)
	{
		UploadRenderingData();
		bDirty = false;
	}
}

FORCEINLINE void SceneComponent::MarkAsDirty()
{
	bDirty = true;
}
