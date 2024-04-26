//
// Created by MarvelLi on 2024/3/29.
//

#pragma once

#include "ActorComponent.h"
#include "Game/World.h"

/**
* RenderingComponent is a base class for components that are used to render or visualize things in the world.
* This component should have methods to upload data to the GPU by interacting with GPU Scene.
* Each RenderingComponent should have a corsponding TransformComponent to store the transform data.
*/
MCLASS(RenderingComponent)
class RenderingComponent : public ActorComponent
{
	REFLECTION_BODY(RenderingComponent)

public:
	RenderingComponent() = default;

	FORCEINLINE Rendering::GPUSceneInterface* GetScene() const { return World->GetScene(); }

	FORCEINLINE virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	FORCEINLINE virtual void TickComponent(double DeltaTime) override;

protected:

	MPROPERTY()
	bool bCastShadow = true;

	MPROPERTY()
	bool bVisible = true;

	virtual void UploadRenderingData(){}

	bool bDirty = false;

	FORCEINLINE void MarkAsDirty();

};

FORCEINLINE void RenderingComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	MarkAsDirty();
}

FORCEINLINE void RenderingComponent::TickComponent(double DeltaTime)
{
	ActorComponent::TickComponent(DeltaTime);
	if (bDirty)
	{
		UploadRenderingData();
		bDirty = false;
	}
}

FORCEINLINE void RenderingComponent::MarkAsDirty()
{
	bDirty = true;
}
