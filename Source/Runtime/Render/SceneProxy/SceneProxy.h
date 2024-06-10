//
// Created by MarvelLi on 2024/4/2.
//

#pragma once
#include <luisa/luisa-compute.h>
#include "Misc/Platform.h"
namespace luisa::compute {
class BindlessArray;
class Accel;
class Stream;
}

namespace MechEngine::Rendering {
class RayTracingScene;
class CameraSceneProxy;
class LightSceneProxy;
class TransformSceneProxy;
class StaticMeshSceneProxy;
}

namespace MechEngine::Rendering
{

// A scene proxy is a data structure that holds the data for collection of a specific type of scene object, like a light or a camera.
// Usually should provide a AddXXX and UpdateXXX function to add or update to the proxy;
// It is used to upload the data to the GPU, and to keep track of what data has changed.
class ENGINE_API SceneProxy
{
public:
	virtual ~SceneProxy() = default;

	explicit SceneProxy(RayTracingScene& InScene);

	virtual bool IsDirty() { return false; }
	virtual void CompileShader() {}
	virtual void UploadDirtyData(luisa::compute::Stream& stream) = 0;
	virtual void PreRenderPass(luisa::compute::Stream& stream) {}
	virtual void PostRenderPass(luisa::compute::Stream& stream) {}

protected:
	template<typename T, typename I>
	[[nodiscard]] auto bindelss_buffer(I &&i) const noexcept { return bindlessArray->buffer<T>(std::forward<I>(i)); }

	RayTracingScene& Scene;
	luisa::compute::Accel& accel;
	luisa::compute::BindlessArray& bindlessArray;
};
}