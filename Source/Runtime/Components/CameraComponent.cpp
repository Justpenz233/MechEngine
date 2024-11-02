#include "CameraComponent.h"
#include "Game/World.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"


void CameraComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	RenderingComponent::PostEdit(Field);
	MarkDirty();
}

void CameraComponent::BeginPlay()
{
	ActorComponent::BeginPlay();
	auto TransformId = GetScene()->GetTransformProxy()->AddTransform(GetOwner()->GetTransformComponent());
	GetScene()->GetCameraProxy()->AddCamera(this, TransformId);
	GetOwner()->GetTransformUpdateDelegate().AddLambda([this]() {
		MarkDirty();
	});
}

void CameraComponent::TickComponent(double DeltaTime)
{
    if(IsDirty)
    {
        UploadRenderingData();
        ClearDirty();
    }
}

void CameraComponent::UploadRenderingData()
{
	GetScene()->GetCameraProxy()->UpdateCamera(this);
}
