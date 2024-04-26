//
// Created by MarvelLi on 2024/3/22.
//

#pragma once
#include "ViewportInterface.h"
#include "Misc/Platform.h"
#include "PointerTypes.h"

namespace MechEngine::Rendering
{
	class GPUSceneInterface;
	class RayTracingScene;
}

/**
 * RenderInterface is the interface for the render system.
 * Renderer should implement this interface to provide rendering functionality.
 * Renderer is responsible for creating and managing the rendering scene corresponding to a world.
 * Provide lifetime function to render a scene.
 */
class RenderInterface
{
public:
	virtual ~RenderInterface() = default;

	virtual Rendering::RayTracingScene* NewScene(class World* InWorld) = 0;

	virtual void EraseScene(Rendering::GPUSceneInterface*) = 0;

	virtual void Init() = 0;

	virtual bool ShouldClose() = 0;

	FORCEINLINE virtual void RenderFrame()
	{
		PreRender();
		Render();
		PostRender();
	}

protected:
	virtual void PreRender() = 0;

	virtual void Render() = 0;

	virtual void PostRender() = 0;
};

/**
 * RenderPipeline is a scene manager along with viewport manager.
 * Technically, this should provide ablitity to manage multiple scenes and viewports.
 */

class RenderPipeline : public RenderInterface
{
public:
	RenderPipeline(int InWidth, int InHeight, const String& InWindowName)
		: Width(InWidth), Height(InHeight), WindowName(InWindowName) {}

	FORCEINLINE ViewportInterface* GetViewport() const
	{
		return Viewport.get();
	}

protected:
	UniquePtr<ViewportInterface> Viewport;
	int Width;
	int Height;
	String WindowName;
};

enum RenderPipelineType
{
	DEFFERED = 0,
	RAYTRACING = 1
};