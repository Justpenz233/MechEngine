//
// Created by MarvelLi on 2024/3/28.
//

#pragma once
#include "PipelineInterface.h"
#include "GpuScene.h"
#include "luisa/luisa-compute.h"

/**
* RenderPipeline is a scene manager along with viewport manager.
* Technically, this should provide ablitity to manage multiple scenes and viewports.
*/

class ENGINE_API RenderPipeline : public PipelineInterface
{
public:
	RenderPipeline(uint width, uint height, const String& title);

	~RenderPipeline() override;

	virtual Rendering::GpuScene* NewScene(class World* InWorld) override;

	virtual void EraseScene(Rendering::GpuSceneInterface*) override;

	virtual void Init() override;

	virtual bool ShouldClose() override;

protected:

	UniquePtr<Rendering::GpuScene> Scene;

	virtual void PreRender() override;

	virtual void Render() override;

	virtual void PostRender() override;

	[[nodiscard]] luisa::compute::Image<float>& FrameBuffer() const noexcept;

public:
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) // Windows
	const String BackEnd = "Dx";
	spdlog::logger logger{"device"};

	#elif __APPLE__ // Macos
	const String BackEnd = "metal";
	#else
	const String BackEnd = "cuda";
	#endif

	FORCEINLINE ViewportInterface* GetViewport() const
	{
		return Viewport.get();
	}

protected:
	luisa::unique_ptr<luisa::compute::ImGuiWindow> MainWindow;
	luisa::compute::Stream Stream{};
	luisa::compute::Device Device{};

	UniquePtr<ViewportInterface> Viewport;
	int Width;
	int Height;
	String WindowName;

	RenderPipelineType RendererType = Null;
};
