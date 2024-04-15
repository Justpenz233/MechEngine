//
// Created by MarvelLi on 2024/3/21.
//

#pragma once
#include "CoreMinimal.h"
#include "ImguiPlugin.h"
#include "igl/opengl/glfw/Viewer.h"
#include "Render/RendererInterface.h"

class StaticMeshComponent;
class TransformComponent;
class World;

class LibiglScene: public GPUSceneInterface
{
public:
	explicit LibiglScene(igl::opengl::glfw::Viewer* InViewer)
		: Viewer(InViewer)
	{
		PointsDataID = Viewer->append_mesh();
		LinesDataID = Viewer->append_mesh();
	}

	void UpdateGPUData();

	bool MeshExist(StaticMeshComponent* InMesh);

	virtual void AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform) override;

	virtual void UpdateStaticMesh(StaticMeshComponent* InMesh) override;

	virtual void EraseMesh(StaticMeshComponent* InMesh) override;

	virtual void UpdateTransform(TransformComponent* InTransform) override;

	virtual void AddCamera(CameraComponent* InCamera, TransformComponent* InTransform) override;

	virtual void UpdateCamera(CameraComponent* InCamera) override;

	virtual void AddLight(LightComponent* InLight, TransformComponent* InTransform) override;

	virtual void UpdateLight(LightComponent* InLight) override;

	// This function should belong to the CameraComponent or Views, but currently we have only one camera
	virtual Matrix4d GetViewMatrix() override;

	virtual Matrix4d GetProjectionMatrix() override;

	virtual void Render() override {}

protected:
	igl::opengl::glfw::Viewer* Viewer;

	THashMap<StaticMeshComponent*, int> MeshIDMap;

	// Mesh is binded to which transform
	THashMap<StaticMeshComponent*, TransformComponent*> TransformMap;

	// Transform is controlling which meshes
	THashMap<TransformComponent*, StaticMeshComponent*> TransformMeshMap;

	// We only have one camera
	CameraComponent* MainCamera = nullptr;
	TransformComponent* CameraTransform = nullptr;

	LightComponent* MainLight = nullptr;
	TransformComponent* LightTransform = nullptr;

	THashSet<TransformComponent*> DirtyTransforms;

	THashSet<StaticMeshComponent*> DirtyMeshes;

	int PointsDataID = -1;

	int LinesDataID = -1;
};

class LibiglPipeline: public RenderPipeline
{
public:
	using RenderPipeline::RenderPipeline;

	virtual void Init() override;

	virtual void PreRender() override;

	virtual void Render() override;

	virtual void PostRender() override;

	virtual void EraseScene(GPUSceneInterface* InScene) override;

	virtual GPUSceneInterface* NewScene(class World* InWorld) override;

	FORCEINLINE virtual bool ShouldClose() override { return bShouldClose; }

protected:
	igl::opengl::glfw::Viewer Viewer;

	bool bShouldClose = false;

	UniquePtr<LibiglScene> Scene;
};
