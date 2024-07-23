//
// Created by MarvelLi on 2024/4/4.
//

#include "StaticMeshSceneProxy.h"

#include "MaterialSceneProxy.h"
#include "TransformProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Render/RayTracing/RayTracingScene.h"

namespace MechEngine::Rendering
{

StaticMeshSceneProxy::StaticMeshSceneProxy(RayTracingScene& InScene)
	: SceneProxy(InScene)
{
	StaticMeshDatas.resize(instance_max_number);
	StaticMeshResource.resize(instance_max_number);
	std::tie(data_buffer, data_buffer_id) = Scene.RegisterBindlessBuffer<static_mesh_data>(instance_max_number);
}
bool StaticMeshSceneProxy::IsDirty()
{
	return !NewMeshes.empty() || !DirtyMeshes.empty() || !DirtyGeometryMeshes.empty();
}

void StaticMeshSceneProxy::AddStaticMesh(StaticMeshComponent* InMesh, uint InTransformID)
{
	ASSERTMSG(!MeshIdMap.count(InMesh), "StaticMeshComponent already exist in scene!");
	if(!Scene.GetTransformProxy()->IsExist(InTransformID))
	{
		LOG_ERROR("Transform Actorname: {} not exist in scene!", InMesh->GetOwnerName());
		return;
	}
	TransformMeshMap[InTransformID] = InMesh;
	MeshIdMap[InMesh] = ~0u; // Mark a temporary index, for the update call at the same frame
	NewMeshes.insert(InMesh);
}

void StaticMeshSceneProxy::UpdateStaticMesh(StaticMeshComponent* InMesh)
{
	DirtyMeshes.insert(InMesh);
}

void StaticMeshSceneProxy::UpdateStaticMeshGeometry(StaticMeshComponent* InMesh)
{
	if (NewMeshes.count(InMesh))
		return;
	DirtyGeometryMeshes.insert(InMesh);
}

void StaticMeshSceneProxy::RemoveStaticMesh(StaticMeshComponent* InMesh)
{
	if(!MeshIdMap.count(InMesh))
	{
		LOG_ERROR("Trying to remove a non-exist mesh: {}", InMesh->GetOwnerName());
		return;
	}

	auto Id = MeshIdMap[InMesh];
	auto Index = IdToIndex[Id];


	// Now swap the last mesh with the mesh to remove
	// auto LastIndex = accel.size() - 1;
	// if (Index != LastIndex)
	// {
	// 	// Get and swap the last mesh data with the mesh to remove
	//
	// 	// Then pop the last mesh
	// 	accel.pop_back();
	// }
	// Scene.destroy(StaticMeshResource[Id].VertexBuffer);
	// Scene.destroy(StaticMeshResource[Id].TriangleBuffer);
	// Scene.destroy(StaticMeshResource[Id].CornerNormalBuffer);
	// Scene.destroy(StaticMeshResource[Id].AccelMesh);
	// StaticMeshResource[Id] = {};
	// StaticMeshDatas[Id] = {};

	accel.set_visibility_on_update(Index, false);
	Scene.GetStream() << accel.build() << synchronize();
	MeshIdMap.erase(InMesh);
}

void StaticMeshSceneProxy::UploadDirtyData(Stream& stream)
{
	bFrameUpdated = false;
	if (!IsDirty()) return;
	auto GetFlattenMeshData = [&](StaticMesh* MeshData)
	{
		int VertexNum = MeshData->GetVertexNum();
		int TriangleNum = MeshData->GetFaceNum();
		vector<Vertex> Vertices(VertexNum);
		vector<Triangle> Triangles(TriangleNum);
		vector<float3> CornerNormals(TriangleNum * 3);
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
		ASSERTMSG(MeshData->CornerNormal.rows() == TriangleNum * 3, "Corner normal size not match! {} != {}",
			MeshData->CornerNormal.rows(), TriangleNum * 3);
		for (int i = 0; i < TriangleNum * 3; i ++)
		{
			CornerNormals[i].x = MeshData->CornerNormal.row(i).x();
			CornerNormals[i].y = MeshData->CornerNormal.row(i).y();
			CornerNormals[i].z = MeshData->CornerNormal.row(i).z();
		}
		return std::tuple{Vertices, Triangles, CornerNormals};
	};
	// Upload new mesh data
	for (auto MeshComponent: NewMeshes)
	{
		auto MeshData = MeshComponent->MeshData;
		if(!MeshData || MeshData->IsEmpty())
		{
			LOG_WARNING("Add an empty mesh to scene: {}", MeshComponent->GetOwnerName());
			continue;
		}
		bFrameUpdated = true;
		auto [Vertices, Triangles, CornerNormals] = GetFlattenMeshData(MeshData.get());
		auto VBuffer = Scene.create<Buffer<Vertex>>(Vertices.size());
		auto TBuffer = Scene.create<Buffer<Triangle>>(Triangles.size());
		auto CornerlNormalBuffer = Scene.create<Buffer<float3>>(CornerNormals.size());
		auto AccelMesh = Scene.create<Mesh>(*VBuffer, *TBuffer, AccelOption{});

		stream << VBuffer->copy_from(Vertices.data())
		<< TBuffer->copy_from(Triangles.data())
		<< CornerlNormalBuffer->copy_from(CornerNormals.data())
		<< commit()
		<< AccelMesh->build();

		auto VBindlessid = Scene.RegisterBindless(VBuffer->view());
		auto TBindlessid = Scene.RegisterBindless(TBuffer->view());
		auto CNBindlessid = Scene.RegisterBindless(CornerlNormalBuffer->view());
		auto MaterialID = Scene.GetMaterialProxy()->AddMaterial(MeshData->GetMaterial());

		accel.emplace_back(*AccelMesh);
		auto Id = ++MeshIdCounter;
		auto InstanceId = accel.size() - 1;
		accel.set_instance_user_id_on_update(InstanceId, Id);
		IdToIndex[Id] = InstanceId;
		MeshIdMap[MeshComponent] = Id;
		StaticMeshDatas[Id].vertex_id = VBindlessid;
		StaticMeshDatas[Id].triangle_id = TBindlessid;
		StaticMeshDatas[Id].material_id = MaterialID;
		StaticMeshDatas[Id].corner_normal_id = CNBindlessid;
		StaticMeshResource[Id] = {AccelMesh, VBuffer, TBuffer, CornerlNormalBuffer};
	}
	if(bFrameUpdated)
	{
		ASSERTMSG(accel.size() <= instance_max_number, "Too many static mesh in scene!");
		stream << data_buffer.subview(0, StaticMeshDatas.size()).copy_from(StaticMeshDatas.data());
	}
	NewMeshes.clear();

	// Update mesh geometry
	for (auto MeshComponent : DirtyGeometryMeshes)
	{
        if(MeshIdMap.count(MeshComponent))
        {
            LOG_WARNING("Trying to update a mesh geometry that not exist in scene: {}", MeshComponent->GetOwnerName());
            continue;
        }

        auto MeshData = MeshComponent->MeshData;
		if(!MeshData || MeshData->IsEmpty())
			continue;
		bFrameUpdated = true;
		auto Id = MeshIdMap[MeshComponent];
		auto Index = IdToIndex[Id];
		auto [Vertices, Triangles, CornerNormals] = GetFlattenMeshData(MeshData.get());

		// Register and upload new data buffer
		auto VBuffer = Scene.create<Buffer<Vertex>>(Vertices.size());
		auto TBuffer = Scene.create<Buffer<Triangle>>(Triangles.size());
		auto CornerlNormalBuffer = Scene.create<Buffer<float3>>(CornerNormals.size());
		auto AccelMesh = Scene.create<Mesh>(*VBuffer, *TBuffer, AccelOption{});
		stream << VBuffer->copy_from(Vertices.data())
		<< TBuffer->copy_from(Triangles.data())
		<< CornerlNormalBuffer->copy_from(CornerNormals.data())
		<< commit()
		<< AccelMesh->build();

		auto VBindlessid = StaticMeshDatas[Id].vertex_id;
		auto TBindlessid = StaticMeshDatas[Id].triangle_id;
		auto CNBindlessid = StaticMeshDatas[Id].corner_normal_id;
		bindlessArray.emplace_on_update(VBindlessid, VBuffer->view());
		bindlessArray.emplace_on_update(TBindlessid, TBuffer->view());
		bindlessArray.emplace_on_update(CNBindlessid, CornerlNormalBuffer->view());
		accel.set_mesh(Index, *AccelMesh);

		// Remove old data buffer
		auto PreVBuffer = StaticMeshResource[MeshIdMap[MeshComponent]].VertexBuffer;
		auto PreTBuffer = StaticMeshResource[MeshIdMap[MeshComponent]].TriangleBuffer;
		auto PreCNBuffer = StaticMeshResource[MeshIdMap[MeshComponent]].CornerNormalBuffer;
		auto PreMesh = StaticMeshResource[MeshIdMap[MeshComponent]].AccelMesh;
		Scene.destroy(PreVBuffer); Scene.destroy(PreTBuffer); Scene.destroy(PreCNBuffer); Scene.destroy(PreMesh);
	}
	DirtyGeometryMeshes.clear();

	// Update mesh visibility
	for (auto MeshComponent: DirtyMeshes)
	{
        if(!MeshIdMap.count(MeshComponent))
        {
            LOG_WARNING("Trying to update a mesh visibility that not exist in scene: {}", MeshComponent->GetOwnerName());
            continue;
        }
        accel.set_visibility_on_update(IdToIndex[MeshIdMap[MeshComponent]], MeshComponent->IsVisible());
		// auto MaterialID = Scene.GetMaterialProxy()->AddMaterial(MeshComponent->GetMeshData()->GetMaterial());
		// StaticMeshDatas[MeshIndexMap[MeshComponent]].material_id = MaterialID;
	}
	DirtyMeshes.clear();
	if(accel.dirty()) stream << accel.build();
}
}