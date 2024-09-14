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
class ENGINE_API RayTracingScene : public GPUSceneInterface
{
public:
	RayTracingScene(Stream& stream, Device& device, luisa::compute::ImGuiWindow* InWindows, ViewportInterface* InViewport) noexcept;

protected:
	luisa::compute::ImGuiWindow* Window;
	ViewportInterface* Viewport;

public:
	void UploadRenderData();

	virtual void Render() override;
	// ---------------------------------------------------------

	[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept override;

	uint2 GetWindosSize() const noexcept;
protected:

	void render_main_view();

	/**
	 * Calculate the color of a pixel with a ray
	 * @param ray the ray to calculate
	 * @param pixel_pos the position of the pixel
	 * @return pixel color
	 */
	Float3 render_pixel(Var<Ray> ray, const Float2& pixel_pos);

	/**
	 * Calculate the color of point in a surface
	 * @param ray the ray cast to the point
	 * @param intersection the intersection point
	 * @param calc_reflection whether the calculation should consider reflection
	 * @return the color of the point, the alpha value of the color
	 */
	std::pair<Float3, Float> calc_surface_point_color(
		Var<Ray> ray, const ray_intersection& intersection, Bool calc_reflection);


	unique_ptr<Shader2D<>> MainShader;

	virtual void CompileShader() override;
};


}
