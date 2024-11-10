//
// Created by MarvelLi on 2024/3/27.
//

#include "GpuScene.h"
#include <luisa/runtime/rtx/accel.h>
#include "Render/Core/ray_tracing_hit.h"
#include "Render/Core/VertexData.h"
#include "Render/ViewportInterface.h"
#include "Render/Core/bxdf_context.h"
#include "Render/Core/math_function.h"
#include "Render/Core/sample.h"
#include "Render/material/shading_function.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/sampler/independent_sampler.h"
#include "Render/sampler/sobol.h"
#include "Render/sampler/sampler_base.h"
namespace MechEngine::Rendering
{

GpuScene::GpuScene(Stream& stream, Device& device, ImGuiWindow* InWindow, ViewportInterface* InViewport) noexcept:
GpuSceneInterface(stream, device), Window(InWindow)
{
	CameraProxy = luisa::make_unique<CameraSceneProxy>(*this);
	LightProxy = luisa::make_unique<LightSceneProxy>(*this);
	TransformProxy = luisa::make_unique<TransformSceneProxy>(*this);
	StaticMeshProxy = luisa::make_unique<StaticMeshSceneProxy>(*this);
	MaterialProxy = luisa::make_unique<MaterialSceneProxy>(*this);
	LineProxy = luisa::make_unique<LineSceneProxy>(*this);
	ShapeProxy = luisa::make_unique<ShapeSceneProxy>(*this);
	Viewport = InViewport;
}

void GpuScene::UploadRenderData()
{
	auto UpdateBindlessArrayIfDirty = [&]() {
		if(bindlessArray.dirty())
		{
			stream << bindlessArray.update();
		}
	};

	// Make sure static mesh data is uploaded before transform data
	// Because need to allocate instance id from accel
	ShapeProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	StaticMeshProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	CameraProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	MaterialProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LightProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	// This may also update transform in accel
	TransformProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	LineProxy->UploadDirtyData(stream);
	UpdateBindlessArrayIfDirty();

	if (rtAccel.dirty()) stream << rtAccel.build() << synchronize();
}

void GpuScene::Render()
{
	stream << (*MainShader)(FrameCounter++, TimeCounter ++).dispatch(GetWindosSize());

	if(ViewMode != ViewMode::FrameBuffer) [[unlikely]]
		stream << (*ViewModePass)(static_cast<uint>(ViewMode)).dispatch(GetWindosSize());

	LineProxy->PostRenderPass(stream);

	stream << synchronize();
}

ImageView<float> GpuScene::frame_buffer() noexcept
{
	return Window->framebuffer();
}

uint2 GpuScene::GetWindosSize() const noexcept
{
	return Window->framebuffer().size();
}

void GpuScene::CompileShader()
{
	// Compile base shaders
	GpuSceneInterface::CompileShader();

	// Main pass shader
	MainShader = luisa::make_unique<Shader2D<uint, uint>>(device.compile<2>(
		[&](UInt frame_index, UInt time) noexcept {
			render_main_view(frame_index, time);
		}));
}

void GpuScene::Init()
{
	GpuSceneInterface::Init();

	LoadRenderSettings();

	InitBuffers();
	InitSamplers();

	CompileShader();
}
void GpuScene::InitBuffers()
{
	auto size = Window->framebuffer().size();
	LOG_DEBUG("GBuffer initial size: {} {}", size.x, size.y);
	g_buffer.base_color = device.create_image<float>(PixelStorage::BYTE4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.normal = device.create_image<float>(PixelStorage::FLOAT4,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.depth = device.create_buffer<float>(size.x * size.y);
	g_buffer.instance_id = device.create_image<uint>(PixelStorage::INT1,
		Window->framebuffer().size().x, Window->framebuffer().size().y);
	g_buffer.frame_buffer = &Window->framebuffer();
	LOG_INFO("Init render frame buffer: {} {}", Window->framebuffer().size().x, Window->framebuffer().size().y);
}



void GpuScene::InitSamplers()
{
	// Initialize the sampler
	sampler = luisa::make_unique<sobol_sampler>(this, stream);
	stream << synchronize();
}

} // namespace MechEngine::Rendering
