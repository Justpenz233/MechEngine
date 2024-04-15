//
// Created by MarvelLi on 2024/4/4.
//

#include "SceneProxy.h"
#include "Render/RayTracing/RayTracingScene.h"
namespace MechEngine::Rendering
{
SceneProxy::SceneProxy(RayTracingScene& InScene)
	: Scene(InScene)
	, accel(InScene.GetAccel()),
	bindlessArray(InScene.getBindlessArray())
{}
}