//
// Created by MarvelLi on 2024/4/4.
//

#include "SceneProxy.h"
#include "Render/PipeLine/GpuScene.h"
namespace MechEngine::Rendering
{
SceneProxy::SceneProxy(GpuScene& InScene)
	: Scene(InScene)
	, accel(InScene.GetAccel()),
	bindlessArray(InScene.GetBindlessArray())
{}
}

