//
// Created by Mayn on 2025/9/27.
//
#include "SceneComponent.h"

#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/TransformProxy.h"

void SceneComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	ActorComponent::PostEdit(Field);
	MarkAsDirty();
	if (Field == NAME(LocalTransform))
	{
		 GetScene()->GetTransformProxy()->UpdateTransform(this);
	}
}
void SceneComponent::SetOwner(Actor* Owner)
{
	ActorComponent::SetOwner(Owner);
	Owner->GetTransformUpdateDelegate().AddMember(this, &SceneComponent::UploadRenderingData);
}
void SceneComponent::UploadRenderingData()
{
	if (TransformId == ~0)
		 TransformId = GetScene()->GetTransformProxy()->AddTransform(this);
	else
		 GetScene()->GetTransformProxy()->UpdateTransform(this);
}
