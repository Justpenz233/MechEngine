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
class ENGINE_API GpuScene : public GpuSceneInterface
{
public:
	GpuScene(Stream& stream, Device& device, luisa::compute::ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

protected:
	luisa::compute::ImGuiWindow* Window;
	ViewportInterface* Viewport;

public:
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

	virtual void Render() override;
	// ---------------------------------------------------------

	[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept override;

	uint2 GetWindosSize() const noexcept;

	[[nodiscard]] auto tone_mapping_uncharted2(Expr<float3> color) noexcept {
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
	
	[[nodiscard]] auto tone_mapping_aces(Expr<float3> color) noexcept {
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


};
}
