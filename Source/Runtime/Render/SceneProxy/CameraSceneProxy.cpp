//
// Created by MarvelLi on 2024/4/2.
//

#include "CameraSceneProxy.h"
#include "Components/CameraComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{

CameraSceneProxy::CameraSceneProxy(GpuScene& InScene)
: SceneProxy(InScene)
{
	std::tie(view_buffer, buffer_id) = Scene.RegisterBindlessBuffer<view>(1);
}

void CameraSceneProxy::AddCamera(CameraComponent* InCameraComponent, uint InTransformID)
{
	MainCameraComponent = InCameraComponent;
	TransformID = InTransformID;
	bDirty = true;
	bFirstFrame = true;
}

void CameraSceneProxy::UpdateCamera(CameraComponent* InCameraComponent)
{
	ASSERTMSG(InCameraComponent == MainCameraComponent, "CameraComponent is not the same");
	bDirty = true;
}

view CameraSceneProxy::GetCurrentViewData() const
{
	auto view_matrix = MainCameraComponent->GetViewMatrix();
	auto projection_matrix = MainCameraComponent->GetProjectionMatrix();
	auto view_projection_matrix = projection_matrix * view_matrix;
	return view{
		.projection_type = 0,
		.aspect_ratio = MainCameraComponent->GetAspectRatio(),
		.tan_half_fovh = MainCameraComponent->GetTanHalfFovH(),
		.tan_half_fovv = MainCameraComponent->GetTanHalfFovV(),

		.viewport_size = Scene.GetWindosSize(),
		.transform_matrix = ToLuisaMatrix(MainCameraComponent->GetOwner()->GetTransformMatrix()),

		.view_matrix = ToLuisaMatrix(view_matrix),
		.inverse_view_matrix = ToLuisaMatrix(view_matrix.inverse().eval()),

		.projection_matrix = ToLuisaMatrix(projection_matrix),
		.inverse_projection_matrix = ToLuisaMatrix(projection_matrix.inverse().eval()),
		
		.view_projection_matrix = ToLuisaMatrix(view_projection_matrix.eval()),
		.inverse_view_projection_matrix = ToLuisaMatrix(view_projection_matrix.inverse().eval()),
	};
}

FMatrix4 CameraSceneProxy::GetViewMatrix(uint Index) const
{
	return MainCameraComponent->GetViewMatrix();
}

FMatrix4 CameraSceneProxy::GetProjectionMatrix(uint Index) const
{
	return MainCameraComponent->GetProjectionMatrix();
}

void CameraSceneProxy::UploadDirtyData(Stream& stream)
{
	if(!MainCameraComponent)
	{
		LOG_ERROR("No valid Camera in scene, raytracing will not work");
		exit(0);
		return;
	}
	if (bDirty)
	{
		PreView = CurrentView;
		CurrentView = GetCurrentViewData();
		CurrentView.last_view_projection_matrix = PreView.view_projection_matrix;
		if(bFirstFrame)
		{
			PreView = CurrentView;
			CurrentView.last_view_projection_matrix = CurrentView.view_projection_matrix;
			bFirstFrame = false;
		}
		stream << view_buffer.copy_from(&CurrentView);
		Scene.ResetFrameCounter();
	}
	bDirty = false;
}

}