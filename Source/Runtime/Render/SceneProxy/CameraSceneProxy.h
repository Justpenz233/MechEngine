//
// Created by MarvelLi on 2024/4/2.
//
#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include "Math/MathType.h"
#include "Render/Core/view.h"
#include "Render/PipeLine/GpuScene.h"

class CameraComponent;
class TransformComponent;

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;

// Currently support only one camera
class ENGINE_API CameraSceneProxy : public SceneProxy
{
public:
	explicit CameraSceneProxy(GpuScene& InScene);

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


	// [[nodiscard]] auto get_camera_position() const { return make_float3(Data->read(0).Transform[3]); }

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

	/**
	 * Get the main view data, used in shader, not support multi view yet
	 * @return Main view data
	 */
	[[nodiscard]] Var<view> get_main_view(/* UInt view_id */) const noexcept { return bindelss_buffer<view>(buffer_id)->read(0); }

protected:

	/** Get main camera's view data */
	[[nodiscard]] view GetCurrentViewData() const;

	bool bFirstFrame = true;
	bool bDirty = true;
	uint buffer_id = ~0u;
	BufferView<view> view_buffer;
	uint TransformID = 0;
	CameraComponent* MainCameraComponent = nullptr;

	view CurrentView;
};
}