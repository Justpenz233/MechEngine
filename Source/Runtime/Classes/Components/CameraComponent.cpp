#include "CameraComponent.h"
#include "Game/World.h"
#include "Render/GpuSceneInterface.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"


void CameraComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	SceneComponent::PostEdit(Field);
	MarkDirty();
}

void CameraComponent::BeginPlay()
{
}

void CameraComponent::UploadRenderingData()
{
	SceneComponent::UploadRenderingData();
	if (bAddedToScene)
		GetScene()->GetCameraProxy()->UpdateCamera(this);
	else
		GetScene()->GetCameraProxy()->AddCamera(this, TransformId);
}
