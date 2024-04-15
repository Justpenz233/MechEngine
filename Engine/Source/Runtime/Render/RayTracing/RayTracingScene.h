//
// Created by MarvelLi on 2024/3/27.
//

#pragma once
#include "Components/LightComponent.h"
#include "Render/RendererInterface.h"
#include "luisa/luisa-compute.h"
#include "Render/Core/ray_tracing_hit.h"
#include "SceneProxy/TransformProxy.h"
#include "SceneProxy/CameraSceneProxy.h"
#include "SceneProxy/LightSceneProxy.h"
#include "SceneProxy/StaticMeshSceneProxy.h"

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
 * The bridge between CPU and GPU data structure is named by lowerCamelCase
 ************************************************************************************/
class RayTracingScene : public GPUSceneInterface
{
public:
	RayTracingScene(Stream& stream, Device& device, luisa::compute::ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

protected:
	class luisa::compute::ImGuiWindow* Window;
	class ViewportInterface* Viewport;

public:

	virtual void AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform) override;

	virtual void UpdateStaticMesh(StaticMeshComponent* InMesh) override;

	virtual void EraseMesh(StaticMeshComponent* InMesh) override;

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

	[[nodiscard]] Image<float>& frame_buffer() const noexcept;

	uint2 GetWindosSize() const noexcept;

	/**
	 * Calculate closest hit information of a ray in the scene.
	 * @param ray Ray to trace
	 * @return Hit information of the closest object hit by the ray
	 */
	[[nodiscard]] ray_tracing_hit trace_closest(const Var<Ray> &ray) const noexcept;

	/**
	 * Test whether a ray has hit any object in the scene.
	 * @param ray Ray to trace
	 * @return Whether the ray has hit any object in the scene
	 */
	[[nodiscard]] Bool has_hit(const Var<Ray> &ray) const noexcept;

	/**
	 * Get the transform data of a transform by id
	 * @param transform_id Transform ID
	 * @return Transform data
	 */
	Var<transformData> get_transform(Expr<uint> transform_id) const noexcept;

	/**
	 * Calaculate the hit position shading information
	 * @param ray Ray to trace
	 * @return Instersection of primitve information of the ray
	 */
	ray_intersection intersect(const Var<Ray> &ray) const noexcept;


	Float4x4 get_instance_transform(Expr<uint> instance_id) const noexcept;

protected:
	Stream& stream;
	Device& device;
	Accel rtAccel;

	vector<unique_ptr<Resource>> Resources;
	static constexpr auto bindless_array_capacity = 500'000u;// limitation of Metal
	static constexpr auto constant_buffer_size = 256u * 1024u;

	//----------- Render primitive data management ------------
	// Mesh collection
	unique_ptr<StaticMeshSceneProxy> StaticMeshProxy;

	// Transform collection
	unique_ptr<TransformSceneProxy> TransformProxy;

	// Light collection
	unique_ptr<LightSceneProxy> LightProxy;

	// Only one camera
	unique_ptr<CameraSceneProxy> CameraProxy;

	BindlessArray bindlessArray;
	size_t _bindless_buffer_count{0u};
	size_t _bindless_tex2d_count{0u};
	size_t _bindless_tex3d_count{0u};

public:
	template<typename T, typename... Args>
	requires std::is_base_of_v<Resource, T>
	[[nodiscard]] auto create(Args &&...args) noexcept -> T * {
		auto resource = luisa::make_unique<T>(device.create<T>(std::forward<Args>(args)...));
		auto p = resource.get();
		Resources.emplace_back(std::move(resource));
		return p;
	}

	template<typename T>
	[[nodiscard]] BufferView<T> RegisterBuffer(size_t n) noexcept {
		return create<Buffer<T>>(n)->view();
	}

	template<typename T>
	[[nodiscard]] auto RegisterBindless(BufferView<T> buffer) noexcept {
		auto buffer_id = _bindless_buffer_count++;
		bindlessArray.emplace_on_update(buffer_id, buffer);
		return static_cast<uint>(buffer_id);
	}

	template<typename T>
	[[nodiscard]] std::pair<BufferView<T>, uint /* bindless id */> RegisterBindlessBuffer(size_t n) noexcept {
		auto view = RegisterBuffer<T>(n);
		auto buffer_id = RegisterBindless(view);
		return std::make_pair(view, buffer_id);
	}

	unique_ptr<Shader2D<>> MainShader;

	Stream& GetStream() noexcept { return stream; }
	Accel& getAccel() { return rtAccel; }
	BindlessArray& getBindlessArray() { return bindlessArray; }
	StaticMeshSceneProxy* GetStaticMeshProxy() { return StaticMeshProxy.get(); }
	TransformSceneProxy* GetTransformProxy() { return TransformProxy.get(); }
	CameraSceneProxy* GetCameraProxy() { return CameraProxy.get(); }
	LightSceneProxy* GetLightProxy() { return LightProxy.get(); }
};


}
