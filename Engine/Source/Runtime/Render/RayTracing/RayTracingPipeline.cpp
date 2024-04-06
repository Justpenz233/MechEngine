//
// Created by MarvelLi on 2024/3/28.
//

#include "RayTracingPipeline.h"
#include "RayTracingScene.h"
#include "Misc/Path.h"
#include <luisa/dsl/syntax.h>
#include "luisa/gui/imgui_window.h"
#include "Render/Core/LuisaViewport.h"

RayTracingPipeline::RayTracingPipeline(uint width, uint height, const String& title)
	:
RenderPipeline(width, height, title)
{
	static luisa::compute::Context Context{Path::BinPath().string()};
	Device        = Context.create_device(BackEnd);
	Stream        = Device.create_stream(luisa::compute::StreamTag::GRAPHICS);
	MainWindow    = luisa::make_unique<luisa::compute::ImGuiWindow>(Device, Stream, luisa::string(title),
		luisa::compute::ImGuiWindow::Config{.size = {width, height}, .resizable = false});
	Viewport = MakeUnique<LuisaViewport>(width, height, this, MainWindow.get(), Stream, Device);
}

GPUSceneInterface* RayTracingPipeline::NewScene(World* InWorld)
{
	Scene = MakeUnique<Rendering::RayTracingScene>(Stream, Device, MainWindow.get(), Viewport.get());
	return Scene.get();
}

void RayTracingPipeline::EraseScene(GPUSceneInterface*)
{

}

void RayTracingPipeline::Init()
{
	MainWindow->prepare_frame();
	MainWindow->render_frame();
	Viewport->LoadViewportStyle();
}

bool RayTracingPipeline::ShouldClose()
{
	return MainWindow->should_close();
}

void RayTracingPipeline::PreRender()
{
	// Prepare frame
	MainWindow->prepare_frame();


	//Reload font if needed
	Viewport->PreFrame();


	// Upload data to GPU
	Scene->UploadData();
}

void RayTracingPipeline::Render()
{
	using namespace luisa::compute;

	if (!MainWindow->framebuffer()) return;

	Scene->Render();

	// Viewport->DrawWidgets();
	Viewport->HandleInput();
}

void RayTracingPipeline::PostRender()
{
	MainWindow->render_frame();
	Viewport->PostFrame();
}

luisa::compute::Image<float>& RayTracingPipeline::FrameBuffer() const noexcept
{
	return MainWindow->framebuffer();
}