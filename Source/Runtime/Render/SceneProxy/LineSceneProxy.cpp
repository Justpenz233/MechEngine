//
// Created by MarvelLi on 2024/4/29.
//

#include "LineSceneProxy.h"
#include "Render/Core/view_data.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{
    LineSceneProxy::LineSceneProxy(RayTracingScene& InScene) noexcept
    : SceneProxy(InScene)
    {
        lines_data_buffer = Scene.RegisterBuffer<lines_data>(MaxCurveCount);
        points_data_buffer = Scene.RegisterBuffer<point_data>(MaxPointCount);
    }

    void LineSceneProxy::CompileShader()
    {
        DrawPointsShader = luisa::make_unique<Shader1D<view_data>>(Scene.RegisterShader<1>(
        [&] (Var<view_data> view)
        {

        }));
    }


    void LineSceneProxy::PostRenderPass(Stream& stream)
    {
        SceneProxy::PostRenderPass(stream);
    }
}
