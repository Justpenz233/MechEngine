//
// Created by MarvelLi on 2024/3/21.
//

#include "LibiglPipeline.h"

#include "Components/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/World.h"
#include "Misc/Config.h"

void LibiglScene::UpdateGPUData()
{
	// Transform all the mesh with their transform

	// First Handle all the dirty transform
	// Insert corresponding mesh into the dirty mesh set
	for(auto& Transform : DirtyTransforms)
	{
		if(TransformMeshMap.contains(Transform))
			DirtyMeshes.insert(TransformMeshMap[Transform]);
	}

	// Update all the dirty mesh
	for(auto Mesh : DirtyMeshes)
	{
		auto MeshID = MeshIDMap[Mesh];
		auto& RenderResource = Viewer->data(MeshIDMap[Mesh]);
		auto MeshData = Mesh->MeshData;
		if(MeshData && !MeshData->IsEmpty())
		{
			if(TransformMap.contains(Mesh))
			{
				MeshData = NewObject<StaticMesh>(*MeshData);
				MeshData->TransformMesh(TransformMap[Mesh]->GetTransformMatrix());
			}
			RenderResource.clear();
			RenderResource.set_mesh(MeshData->verM, MeshData->triM);
			if(MeshData->norM.rows() > 0)
				RenderResource.set_normals(MeshData->norM);
			if(MeshData->colM.rows() > 0)
				RenderResource.set_colors(MeshData->colM);
			RenderResource.id = MeshID;
			RenderResource.set_visible(Mesh->IsVisible());
		}
	}

	DirtyTransforms.clear();
	DirtyMeshes.clear();
}

void LibiglScene::UpdateTransform(TransformComponent* InTransform)
{
	DirtyTransforms.insert(InTransform);
}

void LibiglScene::AddCamera(CameraComponent* InCamera, TransformComponent* InTransform)
{
	MainCamera = InCamera;
	CameraTransform = InTransform;
	// Not support
}

void LibiglScene::UpdateCamera(CameraComponent* InCamera)
{
	Viewer->core().camera_view_angle = InCamera->GetFovH();
	Viewer->core().camera_zoom = InCamera->GetZoom();
}

Matrix4d LibiglScene::GetViewMatrix()
{
	return Viewer->core().view.cast<double>() / Viewer->core().camera_zoom;
}

Matrix4d LibiglScene::GetProjectionMatrix()
{
	return Viewer->core().proj.cast<double>();
}

void LibiglPipeline::Init()
{
	Viewer.core().camera_eye = {-5, 0, 0};
	Viewer.core().camera_up = {0, 0, 1};
	Viewer.core().camera_center = {1, 0, 0};
	Viewer.core().set_rotation_type(igl::opengl::ViewerCore::RotationType::ROTATION_TYPE_TWO_AXIS_VALUATOR_FIXED_UP);
	Viewer.core().is_animating = true;
	Viewer.core().animation_max_fps = 144;
	Viewer.core().light_position = Eigen::Vector3f(0, 0, 0);
	Viewer.core().is_directional_light = false;
	Viewer.core().background_color.setConstant(1.0f);
	auto Plugin = new ImGuiPlugin(Width, Height, this);
	Viewer.plugins.push_back(Plugin);
	Viewport = UniquePtr<ViewportInterface>(Plugin);
	Viewer.launch_init(false, GConfig.Get<String>("Basic", "WindowName"), Width, Height);
}

void LibiglPipeline::PreRender()
{
	Scene->UpdateGPUData();
}

void LibiglPipeline::Render()
{
	bShouldClose = !Viewer.launch_rendering(false);
}

void LibiglPipeline::PostRender()
{

}

bool LibiglScene::MeshExist(StaticMeshComponent* InMesh)
{
	return MeshIDMap.contains(InMesh);
}

void LibiglScene::AddStaticMesh(StaticMeshComponent* InMesh, TransformComponent* InTransform)
{
	auto ID = Viewer->append_mesh(true);
	Viewer->data(ID).show_lines = false;
	Viewer->data(ID).set_face_based(true);
	MeshIDMap[InMesh] = ID;
	TransformMap[InMesh] = InTransform;
	TransformMeshMap[InTransform] = InMesh;
	LOG_DEBUG("Register mesh with ID:{:p} {}",  (void*)(InMesh), ID);
	UpdateStaticMesh(InMesh);
}

void LibiglScene::UpdateStaticMesh(StaticMeshComponent* InMesh)
{
	DirtyMeshes.insert(InMesh);
}

void LibiglScene::EraseMesh(StaticMeshComponent* InMesh)
{
	LOG_DEBUG("Try to erase mesh :{:p}", (void*)(InMesh));
	if (MeshExist(InMesh))
	{
		auto ID = MeshIDMap[InMesh];
		auto Index = Viewer->mesh_index(ID);
		if(Index != -1)
			Viewer->erase_mesh(Index);
		MeshIDMap.erase(InMesh);
	}
	else
	{
		LOG_ERROR("Trying to erase a non-exist mesh");
	}
}

void LibiglPipeline::EraseScene(GPUSceneInterface* InScene)
{
	Scene.reset();
}

GPUSceneInterface* LibiglPipeline::NewScene(class World* InWorld)
{
	Scene = MakeUnique<LibiglScene>(&Viewer);
	return Scene.get();
}