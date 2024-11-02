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
	void UploadRenderData();

	virtual void Render() override;
	// ---------------------------------------------------------

	[[nodiscard]] virtual ImageView<float> frame_buffer() noexcept override;

	uint2 GetWindosSize() const noexcept;
protected:
	/**
	 * Render the main view, dispatch the render kernel
	 * @param frame_index the index of the frame, should start from 0 and increase by 1
	 */
	virtual void render_main_view(const UInt& frame_index, const UInt& time) = 0;

	unique_ptr<Shader2D<uint, uint>> MainShader;

	virtual void CompileShader() override;
};
}
