//
// Created by MarvelLi on 2024/4/29.
//

#include "LineSceneProxy.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
    LineSceneProxy::LineSceneProxy(RayTracingScene& InScene) noexcept
    : SceneProxy(InScene)
    {
        lines_data_buffer = Scene.RegisterBuffer<lines_data>(MaxCurveCount);
    }
}
