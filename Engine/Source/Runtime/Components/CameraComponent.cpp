#include "CameraComponent.h"
#include "Game/World.h"
#include "Render/RendererInterface.h"


void CameraComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	RenderingComponent::PostEdit(Field);
	MarkDirty();
}

void CameraComponent::BeginPlay()
{
	ActorComponent::BeginPlay();
	World->GetScene()->AddCamera(this, GetOwner()->GetTransformComponent());
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
	World->GetScene()->UpdateCamera(this);
}
