//
// Created by MarvelLi on 2024/4/4.
//

#include "StaticMeshSceneProxy.h"

#include "MaterialSceneProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{

StaticMeshSceneProxy::StaticMeshSceneProxy(RayTracingScene& InScene)
: SceneProxy(InScene)
{
	StaticMeshDatas.resize(instance_max_number);
	data_buffer = Scene.RegisterBuffer<staticMeshData>(instance_max_number);
}

void StaticMeshSceneProxy::AddStaticMesh(StaticMeshComponent* InMesh, uint InTransformID)
{
	ASSERTMSG(!MeshIndexMap.count(InMesh), "StaticMeshComponent already exist in scene!");
	TransformMeshMap[InTransformID] = InMesh;
	MeshIndexMap[InMesh] = ~0u; // Mark a temporary index, for the update call at the same frame
	NewMeshes.insert(InMesh);
}

void StaticMeshSceneProxy::UpdateStaticMesh(StaticMeshComponent* InMesh)
{
	ASSERTMSG(MeshIndexMap.count(InMesh), "StaticMeshComponent not exist in scene!");
	DirtyMeshes.insert(InMesh);
}

void StaticMeshSceneProxy::UploadDirtyData(Stream& stream)
{
	if (NewMeshes.empty() && DirtyMeshes.empty()) return;

	// Upload new mesh data
	for (auto MeshComponent: NewMeshes)
	{
		auto MeshData = MeshComponent->MeshData;
		if(!MeshData || MeshData->IsEmpty())
			continue;
		int VertexNum = MeshData->GetVertexNum();
		int TriangleNum = MeshData->GetFaceNum();
		vector<Vertex> Vertices(VertexNum);
		vector<Triangle> Triangles(TriangleNum);
		for (int i = 0; i < VertexNum; i++)
		{
			auto VertexData = MeshData->verM.row(i);
			Vertices[i].px = static_cast<float>(VertexData[0]);
			Vertices[i].py = static_cast<float>(VertexData[1]);
			Vertices[i].pz = static_cast<float>(VertexData[2]);
			auto VertexNormal = MeshData->VertexNormal.row(i);
			Vertices[i].nx = static_cast<float>(VertexNormal[0]);
			Vertices[i].ny = static_cast<float>(VertexNormal[1]);
			Vertices[i].nz = static_cast<float>(VertexNormal[2]);
		}
		for (int i = 0; i < TriangleNum; i++)
		{
			auto FaceData = MeshData->triM.row(i);
			Triangles[i].i0 = FaceData[0];
			Triangles[i].i1 = FaceData[1];
			Triangles[i].i2 = FaceData[2];
		}
		auto VBuffer = Scene.create<Buffer<Vertex>>(uint(VertexNum));
		auto TBuffer = Scene.create<Buffer<Triangle>>(uint(TriangleNum));
		auto AccelMesh = Scene.create<Mesh>(*VBuffer, *TBuffer, AccelOption{});

		stream << VBuffer->copy_from(Vertices.data())
		<< TBuffer->copy_from(Triangles.data()) << commit()
		<< AccelMesh->build();

		auto VBindlessid = Scene.RegisterBindless(VBuffer->view());
		auto TBindlessid = Scene.RegisterBindless(TBuffer->view());
		auto MaterialID = Scene.GetMaterialProxy()->TryAddMaterial(MeshData->GetMaterial());

		accel.emplace_back(*AccelMesh);
		auto Id = accel.size() - 1;
		MeshIndexMap[MeshComponent] = Id;
		StaticMeshDatas[Id].vertex_id = VBindlessid;
		StaticMeshDatas[Id].triangle_id = TBindlessid;
		StaticMeshDatas[Id].material_id = MaterialID;
	}
	if(!NewMeshes.empty())
	{
		ASSERTMSG(accel.size() <= instance_max_number, "Too many static mesh in scene!");
		stream << data_buffer.subview(0, accel.size()).copy_from(StaticMeshDatas.data());
		stream << bindlessArray.update();
	}
	NewMeshes.clear();

	// Update mesh visibility
	for (auto MeshComponent: DirtyMeshes)
	{
		ASSERTMSG(MeshIndexMap[MeshComponent] != ~0u, "StaticMeshComponent not exist in scene!");
		accel.set_visibility_on_update(MeshIndexMap[MeshComponent], MeshComponent->IsVisible());
	}
	DirtyMeshes.clear();
}

}