//
// Created by MarvelLi on 2024/3/22.
//

#pragma once
#include "../ViewportInterface.h"
#include "../../Core/Misc/Platform.h"
#include "../../Core/PointerTypes.h"

enum RenderPipelineType
{
	RealTime = 0,
	PathTracing = 1
};

namespace MechEngine::Rendering
{
	class GpuSceneInterface;
	class GpuScene;
}

/**
 * RenderInterface is the interface for the render system.
 * Renderer should implement this interface to provide rendering functionality.
 * Renderer is responsible for creating and managing the rendering scene corresponding to a world.
 * Provide lifetime function to render a scene.
 */
class PipelineInterface
{
public:
	virtual ~PipelineInterface() = default;

	virtual Rendering::GpuScene* NewScene(class World* InWorld) = 0;

	virtual void EraseScene(Rendering::GpuSceneInterface*) = 0;

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