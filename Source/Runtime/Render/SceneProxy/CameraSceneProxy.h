//
// Created by MarvelLi on 2024/4/2.
//
#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include <luisa/dsl/builtin.h>

#include "Math/MathType.h"
#include "Render/Core/view_data.h"

class CameraComponent;
class TransformComponent;
namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
struct cameraData
{
	uint transformId{};
	float4x4 Transform{};
	float4x4 WorldToView{};
	float4x4 ProjectionMatrix{};
	float AspectRatio{};
	float TanHalfFovH{};
	float TanHalfFovV{};
};
}
LUISA_STRUCT(MechEngine::Rendering::cameraData,
	transformId, Transform, WorldToView, ProjectionMatrix, AspectRatio, TanHalfFovH, TanHalfFovV)
{};

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

// Currently support only one camera
class CameraSceneProxy : public SceneProxy
{
public:
	explicit CameraSceneProxy(RayTracingScene& InScene);

	~CameraSceneProxy() override = default;

	void UploadDirtyData(Stream& stream) override;

	/**
	 * Add a new camera to the scene and bind the corresponding transform
	 * @param InCameraComponent CameraComponent to add
	 * @param InTransformID Corresponding transform ID
	 */
	void AddCamera(CameraComponent* InCameraComponent, uint InTransformID);

	/**
	 * Update the camera data in the scene
	 * @param InCameraComponent CameraComponent to update
	 */
	void UpdateCamera(CameraComponent* InCameraComponent);

	view_data GetCurrentViewData();

	[[nodiscard]] auto generate_ray(Expr<float2> uv) const
	{
		auto p = uv;
		auto cdata = Data->read(0);
		// auto direction      = normalize(make_float3(Near, Near * p.x * TanHalfFovH, Near * p.y * TanHalfFovV));
		auto direction      = normalize(make_float3(1.f, p.x * cdata.TanHalfFovH, p.y * cdata.TanHalfFovV));
		auto origin         = make_float3(cdata.Transform[3]);
		auto world_direction = normalize(make_float3x3(cdata.Transform) * direction);
		return make_ray(make_float3(origin), world_direction);
	}

	[[nodiscard]] auto get_camera_position() const { return make_float3(Data->read(0).Transform[3]); }

	/**
	 * Get the view matrix of camera
	 * @parm Index Index of camera
	 * @return View matrix of camera
	 */
	[[nodiscard]] FMatrix4 GetViewMatrix(uint Index = 0) const;

	/**
	 * Get the projection matrix of camera
	 * @parm Index Index of camera
	 * @return Projection matrix of camera
	 */
	[[nodiscard]] FMatrix4 GetProjectionMatrix(uint Index = 0) const;

protected:
	bool bDirty = true;
	BufferView<cameraData> Data;
	uint TransformID = 0;
	CameraComponent* CameraComponent = nullptr;
};

}