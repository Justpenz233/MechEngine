//
// Created by MarvelLi on 2024/3/27.
//

#pragma once
#include "Render/GPUSceneInterface.h"
#include <luisa/luisa-compute.h>

class ViewportInterface;
class CameraComponent;
class RenderingComponent;


namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
class LuisaViewport;



/************************************************************************************
 * Render scene should hold the ownership of all the render primitives
 * provide the interface for primitives to acess the scene data
 * Nameing Convention:
 * CPU data and methods are names by UpperCamelCase
 * GPU data and methods are names by lower_snake_case
 * The bridge between CPU and GPU data structure is named by lower_snake_case
 ************************************************************************************/
class RayTracingScene : public GPUSceneInterface
{
public:
	RayTracingScene(Stream& stream, Device& device, luisa::compute::ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

protected:
	luisa::compute::ImGuiWindow* Window;
	ViewportInterface* Viewport;

public:
	virtual void AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform) override;

	virtual void UpdateStaticMesh(StaticMeshComponent* InMesh) override;

	virtual void EraseMesh(StaticMeshComponent* InMesh) override;

	virtual void AddMaterial(Material* InMaterial) override;

	virtual void UpdateMaterial(Material* InMaterial) override;

	virtual void AddCamera(CameraComponent* InCamera, TransformComponent* InTransform) override;

	virtual void UpdateCamera(CameraComponent* InCamera) override;

	virtual void UpdateTransform(TransformComponent* InTransform) override;

	virtual void AddLight(LightComponent* InLight, TransformComponent* InTransform) override;

	virtual void UpdateLight(LightComponent* InLight) override;

	virtual Matrix4d GetViewMatrix() override;

	virtual Matrix4d GetProjectionMatrix() override;

	void UploadData();

	virtual void Render() override;
	// ---------------------------------------------------------

	[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept override;

	uint2 GetWindosSize() const noexcept;
protected:
	unique_ptr<Shader2D<view_data, uint>> MainShader;

	virtual void CompileShader() override;
};


}
