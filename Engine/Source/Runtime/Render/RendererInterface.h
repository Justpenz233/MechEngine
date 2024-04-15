//
// Created by MarvelLi on 2024/3/22.
//

#pragma once
#include "ViewportInterface.h"
#include "Misc/Platform.h"
#include "PointerTypes.h"
#include "Math/MathType.h"
#include "ContainerTypes.h"

class RenderingComponent;
class ViewportInterface;
class StaticMeshComponent;
class TransformComponent;
class CameraComponent;
class LightComponent;
class World;

class GPUSceneInterface
{
public:
	virtual  ~GPUSceneInterface() = default;

	GPUSceneInterface() = default;

	/**
	 * Add a new mesh to the scene and bind the corresponding transform
	 * @param InMesh Meshcomponent to add
	 * @param InTransform Corresponding transform
	 */
	virtual void AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform) = 0;

	/**
	 * Update the mesh data in the scene
	 * @param InMesh Meshcomponent to update
	 */
	virtual void UpdateStaticMesh(StaticMeshComponent* InMesh) = 0;

	/**
	 * Erase the mesh from the scene
	 * @param InMesh Meshcomponent to erase
	 */
	virtual void EraseMesh(StaticMeshComponent* InMesh) = 0;

	/**
	 * Update a transform
	 * @param InTransform Transformcomponent to update
	 */
	virtual void UpdateTransform(TransformComponent* InTransform) = 0;

	/**
	 * Add a new camera to the scene and bind the corresponding transform
	 * @param InCamera CameraComponent to add
	 * @param InTransform Corresponding transform
	 */
	virtual void AddCamera(CameraComponent* InCamera, TransformComponent* InTransform) = 0;

	/**
	 * Update the camera data in the scene
	 * @param InCamera CameraComponent to update
	 */
	virtual void UpdateCamera(CameraComponent* InCamera) = 0;

	/**
	 * Add a new light to the scene and bind the corresponding transform
	 * @param InLight LightComponent to add
	 * @param InTransform Corresponding transform
	 */
	virtual void AddLight(LightComponent* InLight, TransformComponent* InTransform) = 0;

	/**
	 * Update the light data in the scene
	 * @param InLight LightComponent to update
	 */
	virtual void UpdateLight(LightComponent* InLight) = 0;

	// virtual void EraseCamera(CameraComponent* InCamera) = 0;

	// /**
	//  * Bind primitive with new corresponding transform
	//  * @param InPrimitive Primitive to bind
	//  * @param InTransform Corresponding transform
	//  */
	// virtual void UpdatePrimitiveTransform(RenderingComponent* InPrimitive, TransformComponent* InTransform) = 0;

	/**
	 * Get the view matrix of the main camera, because we have only one camera in the scene, so can be called main camera
	 * @return View matrix of the main camera
	 */
	virtual FMatrix4 GetViewMatrix() = 0;

	/**
	 * Get the projection matrix of the main camera
	 * @return Projection matrix of the main camera
	 */
	virtual FMatrix4 GetProjectionMatrix() = 0;

	/**
	 * Render the scene
	 */
	virtual void Render() = 0;
};

class RenderInterface
{
public:
	virtual ~RenderInterface() = default;

	virtual GPUSceneInterface* NewScene(class World* InWorld) = 0;

	virtual void EraseScene(GPUSceneInterface*) = 0;

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