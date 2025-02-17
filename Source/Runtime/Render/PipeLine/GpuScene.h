//
// Created by MarvelLi on 2024/3/27.
//

#pragma once
#include "Render/GpuSceneInterface.h"
#include <luisa/luisa-compute.h>

class ViewportInterface;
class CameraComponent;
class RenderingComponent;


namespace MechEngine::Rendering
{
class rasterizer;
class ground_pass;
using namespace luisa;
using namespace luisa::compute;
class LuisaViewport;



/************************************************************************************
 * GpuScene provides the interface of basic scene functions such as intersecting and tone-mapping.
 * Render scene should hold the ownership of all the render primitives
 * provide the interface for primitives to access the scene data
 * Naming Convention:
 * CPU data and methods are names by UpperCamelCase
 * GPU data and methods are names by lower_snake_case
 * The bridge between CPU and GPU data structure is named by lower_snake_case
 ************************************************************************************/
class ENGINE_API GpuScene : public GpuSceneInterface
{
public:
	GpuScene(Stream& stream, Device& device, luisa::compute::ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

	~GpuScene() override;

	/** Create and Init buffers */
	virtual void InitBuffers();

	/** Init and create Sampler */
	virtual void InitSamplers();

	virtual void CompileShader() override;

	/**
	 * Initialize the render scene
	 * Create resources and upload initial data to GPU
	 * Compile shader
	 */
	void Init() override;
	
	void UploadRenderData();

	virtual void PrePass(CommandList& CmdList) {};

	virtual void Render() override;

	/**
	 * Post pass, do some post process after the main pass
	 * Here we implement tone mapping and gamma correction for frame buffer
	 */
	virtual void PostPass(CommandList& CmdList);


public:
/***********************************************************************************************
*									Basic scene shader functions							   *
***********************************************************************************************/

	geometry_buffer& get_gbuffer() { return g_buffer; }

	/**
	 * get the sampler of the thread
	 * @return the sampler of the scene
	 */
	sampler_base* get_sampler() const { return sampler.get(); }

	/**
	 * Calculate the closest hit information of a ray in the scene.
	 * @param ray Ray to trace
	 * @return Hit information of the closest object hit by the ray
	 */
	[[nodiscard]] Var<RayCastHit> trace_closest(const Var<Ray> &ray) const noexcept;

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
	ray_intersection intersect(Var<Ray> ray) const noexcept;
	ray_intersection intersect(Var<RayCastHit> hit, Var<Ray> ray) const noexcept;

	/**
	 * Cast a ray in the scene and return the instance id of the hit object and return the hit to the CPU
	 * @return RayCastHit in CPU
	 */
	RayCastHit RayCastQuery(uint PixelX, uint PixelY);
	TArray<RayCastHit> RayCastQuery(const TArray<uint2>& PixelCoords);

	/**
	* Get the transform data of a transform by instance id
	* @param instance_index instance ID
	* @return Transform matrix
	*/
	Float4x4 get_instance_transform(Expr<uint> instance_index) const noexcept;

	/**
	* Get the transform data of a transform by id
	* @param transform_id Transform ID
	* @return Transform data
	*/
	Var<transform_data> get_transform(Expr<uint> transform_id) const noexcept;

	Var<Triangle> get_triangle(const UInt& instance_id, const UInt& triangle_index) const;

	Var<Vertex> get_vertex(const UInt& instance_id, const UInt& vertex_index) const;

	[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept override;

	uint2 GetWindosSize() const noexcept;

	/**
	 * Calculate the motion vector of the pixel
	 * @param intersection the intersection information of the pixel
	 * @param pixel_coord the coordination of the pixel
	 * @return the motion vector of the pixel
	 */
	[[nodiscard]] Float2 motion_vector(const ray_intersection& intersection) const;

	[[nodiscard]] static auto tone_mapping_uncharted2(Expr<float3> color) noexcept {
		static constexpr auto a = 0.15f;
		static constexpr auto b = 0.50f;
		static constexpr auto c = 0.10f;
		static constexpr auto d = 0.20f;
		static constexpr auto e = 0.02f;
		static constexpr auto f = 0.30f;
		static constexpr auto white = 11.2f;
		auto op = [](auto x) noexcept {
			return (x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f) - e / f;
		};
		return op(1.6f * color) / op(white);
	}

	[[nodiscard]] static auto tone_mapping_aces(Expr<float3> color) noexcept {
		constexpr auto a = 2.51f;
		constexpr auto b = 0.03f;
		constexpr auto c = 2.43f;
		constexpr auto d = 0.59f;
		constexpr auto e = 0.14f;
		return (color * (a * color + b)) / (color * (c * color + d) + e);
	}

protected:

	/**
	 * Render the main view, dispatch the render kernel
	 * @param frame_index the index of the frame, should start from 0 and increase by 1
	 */
	virtual void render_main_view(const UInt& frame_index, const UInt& time) = 0;

	unique_ptr<Shader2D<uint, uint>> MainShader;
	unique_ptr<Shader2D<>> ToneMappingPass;

	// User for ray cast query
	const int MaxQueryCount = 1024;
	unique_ptr<Shader1D<>> RayCastQueryShader;
	BufferView<uint2> RayCastQueryBuffer;
	BufferView<RayCastHit> RayCastHitBuffer;

	unique_ptr<rasterizer> Rasterizer;
	unique_ptr<ground_pass> GroundPass;

	luisa::compute::ImGuiWindow* Window;
	ViewportInterface* Viewport;

};
}
