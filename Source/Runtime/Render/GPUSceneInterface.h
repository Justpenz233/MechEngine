//
// Created by MarvelLi on 2024/4/25.
//

#pragma once
#include "Core/geometry_buffer.h"
#include "Core/VertexData.h"
#include "Core/ViewMode.h"
#include "Core/view_data.h"
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
	struct transform_data;
	class MaterialSceneProxy;
	class LightSceneProxy;
	class CameraSceneProxy;
	class TransformSceneProxy;
	class StaticMeshSceneProxy;
	class ray_tracing_hit;
	class ray_intersection;
	class LineSceneProxy;

	using namespace luisa;
	using namespace luisa::compute;
	/**
	 * GPU scene interface provides the interface to control the rendering scene.
	 * Basically provide interface to register, update, and delete rendering components.
	 * Also should provide interface to manage GPU resources for render pipeline.
	 * Each scene contains a set of stream and device to manage the resources, the renderer should access through the interface.
	 */
	class ENGINE_API GPUSceneInterface
	{
	public:
		GPUSceneInterface(Stream& stream, Device& device) noexcept;

		virtual ~GPUSceneInterface();

		/**
		 * Render the scene
		 */
		virtual void Render() = 0;

		void ViewModeSet(ViewMode InMode) { ViewMode = InMode; }

		[[nodiscard]] ViewMode GetViewMode() const noexcept { return ViewMode; }

		/***********************************************************************************************
		*								Shaders 									                   *
		*					Base shaders for all types pipeline						                   *
		**********************************************************************************************/
	public:
		virtual void CompileShader();


	protected:
		unique_ptr<Shader2D<uint>> ViewModePass;



	/***********************************************************************************************
	*									GPU resource management									   *
	*					Resiger resource and manage ownerships by these interface                  *
	***********************************************************************************************/
	protected:
		ViewMode ViewMode = FrameBuffer;
		Accel rtAccel;
		BindlessArray bindlessArray;
		geometry_buffer g_buffer;

		size_t _bindless_buffer_count{0u};
		size_t _bindless_tex2d_count{0u};
		size_t _bindless_tex3d_count{0u};
		Stream& stream;
		Device& device;

		luisa::vector<luisa::unique_ptr<luisa::compute::Resource>> Resources;
		static constexpr auto bindless_array_capacity = 500'000u;// limitation of Metal
		static constexpr auto constant_buffer_size = 256u * 1024u;

	public:
		/**
		 * Create a resource and register it to the scene
		 * the scene holding the ownership of the resource by unique_ptr and return a raw pointer to the resource
		 * @tparam T Resource type
		 * @tparam Args Resource constructor arguments
		 * @param args Resource constructor arguments
		 * @return Raw pointer to the created resource
		 */
		template <typename T, typename... Args>
		requires std::is_base_of_v<luisa::compute::Resource, T>
		[[nodiscard]] auto create(Args &&...args) noexcept -> T * {
			auto resource = luisa::make_unique<T>(device.create<T>(std::forward<Args>(args)...));
			auto p = resource.get();
			Resources.emplace_back(std::move(resource));
			return p;
		}

		/**
		 * Destroy a resource, free the resource GPU memory
		 * @param resource Resource pointer to destroy
		 * @return Whether the resource is destroyed
		 */
		template<typename T>
		requires std::is_base_of_v<luisa::compute::Resource, T>
		bool destroy(T* resource);


		/***********************************************************************************************
		* 								  Bindless resource create								       *
		*						Provide interface to create bindless resources					       *
		************************************************************************************************/
		template<typename T>
		[[nodiscard]] BufferView<T> RegisterBuffer(size_t n) noexcept {
			return create<Buffer<T>>(n)->view();
		}

		template<typename T>
		[[nodiscard]] auto RegisterBindless(const Image<T> &image, Sampler sampler) noexcept {
			auto tex2d_id = _bindless_tex2d_count++;
			bindlessArray.emplace_on_update(tex2d_id, image, sampler);
			return static_cast<uint>(tex2d_id);
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

		template<uint dim, typename Def>
		auto RegisterShader(Def &&def) noexcept {
			static_assert(dim == 1u || dim == 2u || dim == 3u);
			return device.compile<dim>(std::forward<Def>(def));
		}

		[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept = 0;

		/***********************************************************************************************
		* 								  Bindless resource access								       *
		*						Provide interface to access bindless resources					       *
		************************************************************************************************/

		template<typename T, typename I>
		[[nodiscard]] auto bindelss_buffer(I &&i) const noexcept { return bindlessArray->buffer<T>(std::forward<I>(i)); }

		template<typename I>
		[[nodiscard]] auto bindless_texture_2d(I &&i) const noexcept { return bindlessArray->tex2d(std::forward<I>(i)); }

		template<typename I>
		[[nodiscard]] auto bindless_texture_3d(I &&i) const noexcept { return bindlessArray->tex3d(std::forward<I>(i)); }

		FORCEINLINE Device& GetDevice() noexcept { return device; }
		FORCEINLINE BindlessArray& GetBindlessArray() noexcept { return bindlessArray; }
		FORCEINLINE Accel& GetAccel() { return rtAccel; }
		FORCEINLINE Stream& GetStream() noexcept { return stream; }



	public:

		/***********************************************************************************************
		*									Basic scene shader functions							   *
		*	            				Provide basic query throughout each proxys				       *
		***********************************************************************************************/

		geometry_buffer& get_gbuffer() { return g_buffer; }

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
		* @param view Current view
		* @return Instersection of primitve information of the ray
		*/
		ray_intersection intersect(const Var<Ray>& ray, Var<view_data> view) const noexcept;

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
		Var<transform_data> get_transform(Expr<uint> transform_id) const noexcept;

		Var<Triangle> get_triangle(const UInt& instance_id, const UInt& triangle_index) const;

		Var<Vertex> get_vertex(const UInt& instance_id, const UInt& vertex_index) const;



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
		FORCEINLINE LineSceneProxy* GetLineProxy() { return LineProxy.get(); }

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

		// Line collection
		unique_ptr<LineSceneProxy> LineProxy;

	};

template<typename T>
requires std::is_base_of_v<luisa::compute::Resource, T>
bool GPUSceneInterface::destroy(T* resource)
{
	for (auto iter = Resources.begin(); iter != Resources.end(); ++iter)
	{
		if (iter->get() == resource)
		{
			Resources.erase(iter);
			return true;
		}
	}
	return false;
}


}