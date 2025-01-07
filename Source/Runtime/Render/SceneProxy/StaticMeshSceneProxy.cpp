//
// Created by MarvelLi on 2024/4/4.
//

#include "StaticMeshSceneProxy.h"
#include "MaterialSceneProxy.h"
#include "ShapeSceneProxy.h"
#include "TransformProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/PipeLine/GpuScene.h"
#include "Core/Mesh/StaticMesh.h"

namespace MechEngine::Rendering
{

StaticMeshSceneProxy::StaticMeshSceneProxy(GpuScene& InScene)
	: SceneProxy(InScene)
{
	StaticMeshData.resize(InScene.MaxInstanceNum);
	MeshResources.resize(InScene.MaxInstanceNum);
	MeshInstances.resize(InScene.MaxInstanceNum);
	std::tie(data_buffer, data_buffer_id) = Scene.RegisterBindlessBuffer<static_mesh_data>(InScene.MaxInstanceNum);

	{
		// Create a null mesh
		auto VBuffer = Scene.create<Buffer<Vertex>>(3u);
		auto TBuffer = Scene.create<Buffer<Triangle>>(1u);
		NullMesh = Scene.create<Mesh>(*VBuffer, *TBuffer, AccelOption{});
	}
}

bool StaticMeshSceneProxy::IsDirty()
{
	return !CommandQueue.empty();
}

uint StaticMeshSceneProxy::AddStaticMesh(StaticMesh* InMesh)
{
	if (!InMesh || InMesh->IsEmpty())
	{
		LOG_WARNING("Trying to add a null mesh to the scene.");
		return ~0u;
	}
	auto Id = ++MeshIdCounter;
	CommandQueue.emplace_back(Create, Id, 0, InMesh);
	return Id;
}

void StaticMeshSceneProxy::UpdateStaticMeshGeometry(uint MeshId, StaticMesh* InMesh)
{
	if (!InMesh || InMesh->IsEmpty() || MeshId > MeshIdCounter)
	{
		LOG_WARNING("Trying to update a null mesh to the scene.");
		return;
	}
	CommandQueue.emplace_back(Update, MeshId, 0, InMesh);
}

void StaticMeshSceneProxy::BindInstance(uint MeshID, uint InstanceID)
{
	if (MeshID > MeshIdCounter)
	{
		LOG_WARNING("Trying to bind a mesh that does not exist. ID {}", MeshID);
		return;
	}
	CommandQueue.emplace_back(Bind, MeshID, InstanceID, nullptr);
}

void StaticMeshSceneProxy::RemoveStaticMesh(uint MeshId)
{
	CommandQueue.emplace_back(Delete, MeshId, 0, nullptr);
}

void StaticMeshSceneProxy::UploadDirtyData(Stream& stream)
{
	bFrameUpdated = false;

	static std::once_flag init_null_mesh_flag;
	std::call_once(init_null_mesh_flag, [&]()
	{
		stream << NullMesh->build();
	});

	for (auto& Command : CommandQueue)
	{
		auto Type = std::get<0>(Command);
		auto Id1 = std::get<1>(Command);
		auto Id2 = std::get<2>(Command);
		auto MeshPtr = std::get<3>(Command);
		switch (Type)
		{
			case Create:
			{
				if (!MeshPtr || MeshPtr->IsEmpty())
				{
					LOG_ERROR("Add an empty mesh to scene: {}", MeshPtr->GetName());
					continue;
				}
				bFrameUpdated = true;
				MeshIdToPtr[Id1] = MeshPtr;
				auto [Vertices, Triangles, CornerNormals] = GetFlattenMeshData(MeshPtr);
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
				auto MaterialID = Scene.GetMaterialProxy()->AddMaterial(MeshPtr->GetMaterial());
				StaticMeshData[Id1] = { VBindlessid, TBindlessid, CNBindlessid, MaterialID, 0 };
				MeshResources[Id1] = { AccelMesh, VBuffer, TBuffer, CornerlNormalBuffer };
				break;
			}
			case Update:
			{
				if (!MeshPtr || MeshPtr->IsEmpty())
				{
					LOG_ERROR("Update an empty mesh to scene: {}", MeshPtr->GetName());
					continue;
				}
				bFrameUpdated = true;
				MeshIdToPtr[Id1] = MeshPtr;
				auto [Vertices, Triangles, CornerNormals] = GetFlattenMeshData(MeshPtr);

				// Register and upload new data buffer
				auto VBuffer = Scene.create<Buffer<Vertex>>(Vertices.size());
				auto TBuffer = Scene.create<Buffer<Triangle>>(Triangles.size());
				auto CornerNormalBuffer = Scene.create<Buffer<float3>>(CornerNormals.size());
				auto AccelMesh = Scene.create<Mesh>(*VBuffer, *TBuffer, AccelOption{});
				stream << VBuffer->copy_from(Vertices.data())
					   << TBuffer->copy_from(Triangles.data())
					   << CornerNormalBuffer->copy_from(CornerNormals.data())
					   << commit()
					   << AccelMesh->build();

				auto VBindlessid = StaticMeshData[Id1].vertex_buffer_id;
				auto TBindlessid = StaticMeshData[Id1].triangle_buffer_id;
				auto CNBindlessid = StaticMeshData[Id1].corner_normal_buffer_id;
				auto MaterialID = Scene.GetMaterialProxy()->AddMaterial(MeshPtr->GetMaterial());

				ASSERTMSG(VBindlessid != ~0u && TBindlessid != ~0u && CNBindlessid != ~0u,
					"Trying to update a mesh that does not exist. ID {}", Id1);
				bindlessArray.emplace_on_update(VBindlessid, VBuffer->view());
				bindlessArray.emplace_on_update(TBindlessid, TBuffer->view());
				bindlessArray.emplace_on_update(CNBindlessid, CornerNormalBuffer->view());

				for (auto Instance : MeshInstances[Id1])
				{
					accel.set_mesh(Instance, *AccelMesh);
					Scene.GetShapeProxy()->SetInstanceMeshID(Instance, Id1);
				}

				auto PreVBuffer = MeshResources[Id1].VertexBuffer;
				auto PreTBuffer = MeshResources[Id1].TriangleBuffer;
				auto PreCNBuffer = MeshResources[Id1].CornerNormalBuffer;
				auto PreMesh = MeshResources[Id1].AccelMesh;
				Scene.destroy(PreVBuffer);
				Scene.destroy(PreTBuffer);
				Scene.destroy(PreCNBuffer);
				Scene.destroy(PreMesh);

				StaticMeshData[Id1] = { VBindlessid, TBindlessid, CNBindlessid, MaterialID, 0};
				MeshResources[Id1] = { AccelMesh, VBuffer, TBuffer, CornerNormalBuffer };
				break;
			}
			case Delete:
			{
				for (auto Instance : MeshInstances[Id1])
				{
					accel.set_visibility_on_update(Instance, false);
					// accel.set_mesh(Instance, *NullMesh);
				}
				// stream << accel.build();

				// bindlessArray.remove_buffer_on_update(StaticMeshData[Id1].vertex_buffer_id);
				// bindlessArray.remove_buffer_on_update(StaticMeshData[Id1].triangle_buffer_id);
				// bindlessArray.remove_buffer_on_update(StaticMeshData[Id1].corner_normal_buffer_id);
				// stream << bindlessArray.update();

				// Scene.destroy(MeshResources[Id1].AccelMesh);
				// Scene.destroy(MeshResources[Id1].VertexBuffer);
				// Scene.destroy(MeshResources[Id1].TriangleBuffer);
				// Scene.destroy(MeshResources[Id1].CornerNormalBuffer);
				MeshResources[Id1] = {};
				MeshIdToPtr.erase(Id1);
				break;
			}
			case Bind:
			{
				if (!MeshResources[Id1].AccelMesh)
				{
					LOG_ERROR("Trying to bind a mesh that does not exist. ID {}", Id1);
					continue;
				}
				for (auto Meshes : MeshInstances)
					if (Meshes.count(Id2)) Meshes.erase(Id2);
				MeshInstances[Id1].insert(Id2);
				accel.set_mesh(Id2, *MeshResources[Id1].AccelMesh);
			}
		}
	}
	CommandQueue.clear();

	if (bFrameUpdated)
		stream << data_buffer.subview(0, StaticMeshData.size()).copy_from(StaticMeshData.data());
}
std::tuple<vector<Vertex>, vector<Triangle>, vector<float3>> StaticMeshSceneProxy::GetFlattenMeshData(StaticMesh* MeshData)
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
	if(MeshData->HasValidUV())
	{
		auto UVData = MeshData->GetUV();
		for (int i = 0; i < UVData.rows(); i++)
		{
			Vertices[i].u = static_cast<float>(UVData.row(i).x());
			Vertices[i].v = static_cast<float>(UVData.row(i).y());
		}
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
}

} // namespace MechEngine::Rendering