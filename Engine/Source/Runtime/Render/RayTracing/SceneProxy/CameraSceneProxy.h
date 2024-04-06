//
// Created by MarvelLi on 2024/4/2.
//
#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include <luisa/dsl/builtin.h>

#include "Math/MathType.h"

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
	CameraSceneProxy(RayTracingScene& InScene);
	~CameraSceneProxy() override = default;

	void UploadDirtyData(Stream& stream) override;

	void AddCamera(CameraComponent* InCameraComponent, uint InTransformID);

	void UpdateCamera(CameraComponent* InCameraComponent);

	auto GenerateRay(Expr<float2> uv)
	{
		auto p = uv;
		auto cdata = Data->read(0);
		// auto direction      = normalize(make_float3(Near, Near * p.x * TanHalfFovH, Near * p.y * TanHalfFovV));
		auto direction      = normalize(make_float3(1.f, p.x * cdata.TanHalfFovH, p.y * cdata.TanHalfFovV));
		auto origin         = make_float3(cdata.Transform[3]);
		auto world_direction = normalize(make_float3x3(cdata.Transform) * direction);
		return make_ray(make_float3(origin), world_direction);
	}

	auto GetPosition() { return make_float3(Data->read(0).Transform[3]); }

	FMatrix4 GetViewMatrix(uint Index = 0) const;
	FMatrix4 GetProjectionMatrix(uint Index = 0) const;

protected:
	bool bDirty = true;
	BufferView<cameraData> Data;
	uint TransformID = 0;
	CameraComponent* CameraComponent = nullptr;
};

}