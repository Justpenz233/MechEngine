//
// Created by MarvelLi on 2024/4/25.
//

#pragma once
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include "Math/MathType.h"

class Material;
class StaticMeshComponent;
class TransformComponent;
class CameraComponent;
class LightComponent;


namespace MechEngine::Rendering
{
	struct transformData;
	class MaterialSceneProxy;
	class LightSceneProxy;
	class CameraSceneProxy;
	class TransformSceneProxy;
	class StaticMeshSceneProxy;
	class ray_tracing_hit;
	class ray_intersection;

	using namespace luisa;
	using namespace luisa::compute;


	/**
	 * GPU scene interface provides the interface to control the rendering scene.
	 * Basically provide interface to register, update, and delete rendering components.
	 * Also should provide interface to manage GPU resources for render pipeline.
	 * Each scene contains a set of stream and device to manage the resources, the renderer should access through the interface.
	 */
	class GPUSceneInterface
	{
	public:
		GPUSceneInterface(Stream& stream, Device& device) noexcept;

		virtual ~GPUSceneInterface();

		/**
		 * Add a new mesh to the scene and bind the corresponding transform
		 * @param InMesh Meshcomponent to add
		 * @param InTransform Corresponding transform
		 */
		virtual void AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform) = 0;

		/**
		 * Update the mesh data in the scene
		 * @param InMesh Meshcomponent to update
		 */
		virtual void UpdateStaticMesh(StaticMeshComponent* InMesh) = 0;

		/**
		 * Add a new material to the scene
		 * @param InMaterial Material to add
		 */
		virtual void AddMaterial(Material* InMaterial) = 0;

		/**
		 * Update the material data in the scene
		 * @param InMaterial Material to update
		 */
		virtual void UpdateMaterial(Material* InMaterial) = 0;

		/**
		 * Erase the mesh from the scene
		 * @param InMesh Meshcomponent to erase
		 */
		virtual void EraseMesh(StaticMeshComponent* InMesh) = 0;

		/**
		 * Update a transform
		 * @param InTransform Transformcomponent to update
		 */
		virtual void UpdateTransform(TransformComponent* InTransform) = 0;

		/**
		 * Add a new camera to the scene and bind the corresponding transform
		 * @param InCamera CameraComponent to add
		 * @param InTransform Corresponding transform
		 */
		virtual void AddCamera(CameraComponent* InCamera, TransformComponent* InTransform) = 0;

		/**
		 * Update the camera data in the scene
		 * @param InCamera CameraComponent to update
		 */
		virtual void UpdateCamera(CameraComponent* InCamera) = 0;

		/**
		 * Add a new light to the scene and bind the corresponding transform
		 * @param InLight LightComponent to add
		 * @param InTransform Corresponding transform
		 */
		virtual void AddLight(LightComponent* InLight, TransformComponent* InTransform) = 0;

		/**
		 * Update the light data in the scene
		 * @param InLight LightComponent to update
		 */
		virtual void UpdateLight(LightComponent* InLight) = 0;

		// virtual void EraseCamera(CameraComponent* InCamera) = 0;

		// /**
		//  * Bind primitive with new corresponding transform
		//  * @param InPrimitive Primitive to bind
		//  * @param InTransform Corresponding transform
		//  */
		// virtual void UpdatePrimitiveTransform(RenderingComponent* InPrimitive, TransformComponent* InTransform) = 0;

		/**
		 * Get the view matrix of the main camera, because we have only one camera in the scene, so can be called main camera
		 * @return View matrix of the main camera
		 */
		virtual FMatrix4 GetViewMatrix() = 0;

		/**
		 * Get the projection matrix of the main camera
		 * @return Projection matrix of the main camera
		 */
		virtual FMatrix4 GetProjectionMatrix() = 0;

		/**
		 * Render the scene
		 */
		virtual void Render() = 0;



	/***********************************************************************************************
	*									GPU resource management									   *
	*					Resiger resource and manage ownerships by these interface                  *
	***********************************************************************************************/
	protected:
		luisa::compute::Accel rtAccel;
		BindlessArray bindlessArray;
		size_t _bindless_buffer_count{0u};
		size_t _bindless_tex2d_count{0u};
		size_t _bindless_tex3d_count{0u};
		Stream& stream;
		Device& device;

		luisa::vector<luisa::unique_ptr<luisa::compute::Resource>> Resources;
		static constexpr auto bindless_array_capacity = 500'000u;// limitation of Metal
		static constexpr auto constant_buffer_size = 256u * 1024u;

	public:
		template<typename T, typename... Args>
		requires std::is_base_of_v<luisa::compute::Resource, T>
		[[nodiscard]] auto create(Args &&...args) noexcept -> T * {
			auto resource = luisa::make_unique<T>(device.create<T>(std::forward<Args>(args)...));
			auto p = resource.get();
			Resources.emplace_back(std::move(resource));
			return p;
		}

		template<typename T>
		[[nodiscard]] luisa::compute::BufferView<T> RegisterBuffer(size_t n) noexcept {
			return create<luisa::compute::Buffer<T>>(n)->view();
		}

		template<typename T>
		[[nodiscard]] auto RegisterBindless(luisa::compute::BufferView<T> buffer) noexcept {
			auto buffer_id = _bindless_buffer_count++;
			bindlessArray.emplace_on_update(buffer_id, buffer);
			return static_cast<uint>(buffer_id);
		}

		template<typename T>
		[[nodiscard]] std::pair<luisa::compute::BufferView<T>, uint /* bindless id */> RegisterBindlessBuffer(size_t n) noexcept {
			auto view = RegisterBuffer<T>(n);
			auto buffer_id = RegisterBindless(view);
			return std::make_pair(view, buffer_id);
		}

		FORCEINLINE Accel& GetAccel() { return rtAccel; }
		FORCEINLINE Stream& GetStream() noexcept { return stream; }
		FORCEINLINE BindlessArray& getBindlessArray() { return bindlessArray; }



	public:

		/***********************************************************************************************
		*									Basic scene shader functions							   *
		*	            				Provide basic query throughout each proxys				       *
		***********************************************************************************************/

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
		 * Calaculate the hit position shading information
		* @param ray Ray to trace
		* @return Instersection of primitve information of the ray
		*/
		ray_intersection intersect(const Var<Ray> &ray) const noexcept;

		/**
		* Get the transform data of a transform by instance id
		* @param instance_id instance ID
		* @return Transform matrix
		*/
		Float4x4 get_instance_transform(Expr<uint> instance_id) const noexcept;


		/**
		* Get the transform data of a transform by id
		* @param transform_id Transform ID
		* @return Transform data
		*/
		Var<transformData> get_transform(Expr<uint> transform_id) const noexcept;



		/***********************************************************************************************
		 *								Scene proxy management									       *
		 *					  Each scene proxy is a collection of same type rendering components	   *
		 *				Provide interface to access render components' GPU data						   *
		 ***********************************************************************************************/

	public:
		FORCEINLINE StaticMeshSceneProxy* GetStaticMeshProxy() { return StaticMeshProxy.get(); }
		FORCEINLINE TransformSceneProxy* GetTransformProxy() { return TransformProxy.get(); }
		FORCEINLINE CameraSceneProxy* GetCameraProxy() { return CameraProxy.get(); }
		FORCEINLINE LightSceneProxy* GetLightProxy() { return LightProxy.get(); }
		FORCEINLINE MaterialSceneProxy* GetMaterialProxy() { return MaterialProxy.get(); }


	protected:
		// Mesh collection
		unique_ptr<StaticMeshSceneProxy> StaticMeshProxy;

		// Transform collection
		unique_ptr<TransformSceneProxy> TransformProxy;

		// Light collection
		unique_ptr<LightSceneProxy> LightProxy;

		// Only one camera
		unique_ptr<CameraSceneProxy> CameraProxy;

		// Material collection
		unique_ptr<MaterialSceneProxy> MaterialProxy;

	};
}