//
// Created by MarvelLi on 2024/4/25.
//

#include "GpuSceneInterface.h"
#include "Core/RayCastHit.h"
#include "Core/VertexData.h"
#include "Core/view.h"
#include "Misc/Config.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/LightSceneProxy.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/SceneProxy/LineSceneProxy.h"
#include "SceneProxy/ShapeSceneProxy.h"
#include "sampler/sampler_base.h"

namespace MechEngine::Rendering
{
    GpuSceneInterface::GpuSceneInterface(Stream& stream, Device& device) noexcept
    : stream(stream), device(device)
    {
        rtAccel = device.create_accel({});
        bindlessArray = device.create_bindless_array(bindless_array_capacity);
        // !Proxies should be created in the derived class, case the proxies may be polymorphic for different renderers
    }

    // empty detor here to make unqiue ptr happy
    GpuSceneInterface::~GpuSceneInterface() = default;

	void GpuSceneInterface::ResetFrameCounter() noexcept
	{
    	FrameCounter = 0;
	}

	void GpuSceneInterface::LoadRenderSettings()
	{
    	bShadowRayOffset = GConfig.Get<bool>("Render", "ShadowRayOffset");
    	SamplePerPixel = GConfig.Get<int>("Render", "SamplePerPixel");
		bHDR = GConfig.Get<bool>("Render", "HDR");
		bShaderDebugInfo = GConfig.Get<bool>("RenderDebug", "ShaderDebugInfo");
		bUseRasterizer = GConfig.Get<bool>("DeferredShading", "UseRasterizer");
	}
}
