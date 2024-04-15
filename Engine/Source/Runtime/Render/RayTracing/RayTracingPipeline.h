//
// Created by MarvelLi on 2024/3/28.
//

#pragma once
#include "Render/RendererInterface.h"
#include "RayTracingScene.h"
#include "luisa/luisa-compute.h"

class RayTracingPipeline : public RenderPipeline
{
public:
	RayTracingPipeline(uint width, uint height, const String& title);

	virtual ~RayTracingPipeline() override;

	virtual GPUSceneInterface* NewScene(class World* InWorld) override;

	virtual void EraseScene(GPUSceneInterface*) override;

	virtual void Init() override;

	virtual bool ShouldClose() override ;

protected:

	UniquePtr<Rendering::RayTracingScene> Scene;

	virtual void PreRender() override;

	virtual void Render() override;

	virtual void PostRender() override;

	[[nodiscard]] luisa::compute::Image<float>& FrameBuffer() const noexcept;

public:
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) // Windows
	const String BackEnd = "Dx";
	#elif __APPLE__ // Macos
	const String BackEnd = "Metal";
	#else
	const String BackEnd = "Cuda";
	#endif


protected:
	luisa::unique_ptr<luisa::compute::ImGuiWindow> MainWindow;
	luisa::compute::Stream Stream{};
	luisa::compute::Device Device{};
};
