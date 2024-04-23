//
// Created by MarvelLi on 2024/4/2.
//

#pragma once

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
class SceneProxy
{
public:
	virtual ~SceneProxy() = default;

	explicit SceneProxy(RayTracingScene& InScene);

	virtual void UploadDirtyData(luisa::compute::Stream& stream) = 0;

protected:
	RayTracingScene& Scene;
	luisa::compute::Accel& accel;
	luisa::compute::BindlessArray& bindlessArray;
};
}