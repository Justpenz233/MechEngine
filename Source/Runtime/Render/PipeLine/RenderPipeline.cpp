//
// Created by MarvelLi on 2024/3/28.
//

#include "RenderPipeline.h"

#include "DeferredShadingScene.h"
#include "GpuScene.h"
#include "Misc/Path.h"
#include "luisa/gui/imgui_window.h"
#include "Render/Core/LuisaViewport.h"
#include "Render/PipeLine/PathTracingScene.h"

RenderPipeline::RenderPipeline(uint width, uint height, const String& title)
: Width(width), Height(height), WindowName(title)
{
	static luisa::compute::Context Context{Path::BinPath().string()};
	Device        = Context.create_device(BackEnd);
	Stream        = Device.create_stream(luisa::compute::StreamTag::GRAPHICS);
	MainWindow    = luisa::make_unique<luisa::compute::ImGuiWindow>(Device, Stream, luisa::string(title),
		luisa::compute::ImGuiWindow::Config{.size = {width, height}, .resizable = false, .hdr = GConfig.Get<bool>("Render", "HDR"), .multi_viewport = false});
	Viewport = MakeUnique<LuisaViewport>(width, height, this, MainWindow.get(), Stream, Device);

	RendererType = static_cast<RenderPipelineType>(GConfig.Get<int>("Render", "RenderPipelineType"));

	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) // Windows
		logger = {"device", std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
		Stream.set_log_callback([&](luisa::string_view message) mutable noexcept {
			if (!message.empty()) {
				logger.info("{}", message);
			}
		});
	#endif
}

RenderPipeline::~RenderPipeline()
{
	// Release viewport first so ImGui context is still valid
	Viewport.reset();

	// Destroy main window
	MainWindow->destroy();
}

Rendering::GpuScene* RenderPipeline::NewScene(World* InWorld)
{
	switch (RendererType)
	{
		case PathTracing:
		{
			Scene = MakeUnique<Rendering::PathTracingScene>(Stream, Device, MainWindow.get(), Viewport.get());
			break;
		}
		case DeferredShading:
		{
			Scene = MakeUnique<Rendering::DeferredShadingScene>(Stream, Device, MainWindow.get(), Viewport.get());
			break;
		}
		default:
		{
			LOG_ERROR("Not support");
		}
	}
	Scene->Init();
	return Scene.get();
}

void RenderPipeline::EraseScene(Rendering::GpuSceneInterface*)
{

}

void RenderPipeline::Init()
{
	if (BackEnd == "Dx")
	{
		Stream.set_log_callback([](luisa::string_view str) {
			LOG_TEMP(str);
		});
	}
	MainWindow->prepare_frame();
	MainWindow->render_frame();
	Viewport->LoadViewportStyle();
}

bool RenderPipeline::ShouldClose()
{
	return MainWindow->should_close();
}

void RenderPipeline::PreRender()
{
	// Prepare frame
	MainWindow->prepare_frame();


	//Reload font if needed
	Viewport->PreFrame();


	// Upload data to GPU
	Scene->UploadRenderData();
}

void RenderPipeline::Render()
{
	using namespace luisa::compute;

	if (!MainWindow->framebuffer()) return;

	Scene->Render();

	Viewport->DrawWidgets();
	Viewport->HandleInput();
}

void RenderPipeline::PostRender()
{
	MainWindow->render_frame();
	Viewport->PostFrame();
}

luisa::compute::Image<float>& RenderPipeline::FrameBuffer() const noexcept
{
	return MainWindow->framebuffer();
}