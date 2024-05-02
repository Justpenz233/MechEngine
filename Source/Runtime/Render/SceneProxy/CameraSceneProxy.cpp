//
// Created by MarvelLi on 2024/4/2.
//

#include "CameraSceneProxy.h"
#include "Components/CameraComponent.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{

CameraSceneProxy::CameraSceneProxy(RayTracingScene& InScene)
: SceneProxy(InScene)
,Data {InScene.RegisterBuffer<cameraData>(1)}
{}

void CameraSceneProxy::AddCamera(::CameraComponent* InCameraComponent, uint InTransformID)
{
	CameraComponent = InCameraComponent;
	TransformID = InTransformID;
	bDirty = true;
}

void CameraSceneProxy::UpdateCamera(::CameraComponent* InCameraComponent)
{
	ASSERTMSG(InCameraComponent == CameraComponent, "CameraComponent is not the same");
	bDirty = true;
}

view_data CameraSceneProxy::GetCurrentViewData()
{
	auto view_matrix = CameraComponent->GetViewMatrix();
	auto projection_matrix = CameraComponent->GetProjectionMatrix();
	auto view_projection_matrix = projection_matrix * view_matrix;
	return view_data{
		.projection_type = 0,
		.aspect_ratio = CameraComponent->GetAspectRatio(),
		.fov_h =  CameraComponent->GetFovH(),
		.viewport_size = Scene.GetWindosSize(),
		.view_matrix = ToLuisaMatrix(view_matrix),
		.projection_matrix = ToLuisaMatrix(projection_matrix),
		.inverse_view_matrix = ToLuisaMatrix(view_matrix.inverse().eval()),
		.inverse_projection_matrix = ToLuisaMatrix(projection_matrix.inverse().eval()),
		.view_projection_matrix = ToLuisaMatrix(view_projection_matrix.eval()),
		.inverse_view_projection_matrix = ToLuisaMatrix(view_projection_matrix.inverse().eval()),
	};
}

FMatrix4 CameraSceneProxy::GetViewMatrix(uint Index) const
{
	return CameraComponent->GetViewMatrix();
}

FMatrix4 CameraSceneProxy::GetProjectionMatrix(uint Index) const
{
	return CameraComponent->GetProjectionMatrix();
}

void CameraSceneProxy::UploadDirtyData(Stream& stream)
{
	if(!CameraComponent)
	{
		LOG_ERROR("No valid Camera in scene, raytracing will not work");
		exit(0);
		return;
	}
	if (bDirty)
	{
		cameraData data;
		data.transformId = TransformID;
		data.Transform = ToLuisaMatrix(CameraComponent->GetOwner()->GetTransformMatrix());
		data.WorldToView = ToLuisaMatrix(CameraComponent->GetViewMatrix());
		data.ProjectionMatrix = ToLuisaMatrix(CameraComponent->GetProjectionMatrix());
		data.AspectRatio = CameraComponent->GetAspectRatio();
		data.TanHalfFovH = std::tan(0.5f * radians(CameraComponent->GetFovH()));
		data.TanHalfFovV = data.TanHalfFovH / data.AspectRatio;
		stream << Data.copy_from(&data);
	}
	bDirty = false;
}

}