//
// Created by MarvelLi on 2024/4/4.
//

#include "StaticMeshSceneProxy.h"
#include "MaterialSceneProxy.h"
#include "TransformProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Mesh/BasicShapesLibrary.h"
#include "Render/RayTracing/RayTracingScene.h"
#include "Core/Mesh/StaticMesh.h"

namespace MechEngine::Rendering
{

StaticMeshSceneProxy::StaticMeshSceneProxy(RayTracingScene& InScene)
	: SceneProxy(InScene)
{
	StaticMeshData.resize(instance_max_number);
	MeshResources.resize(instance_max_number);
	MeshInstances.resize(instance_max_number);
	std::tie(data_buffer, data_buffer_id) = Scene.RegisterBindlessBuffer<static_mesh_data>(instance_max_number);
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
	CommandQueue.emplace_back(CMesh, Id, 0, InMesh);
	return Id;
}

uint StaticMeshSceneProxy::AddInstance(uint MeshId, uint TransformId)
{
	accel.emplace_back(0);
	auto InstanceId = accel.size() - 1;
	CommandQueue.emplace_back(CInstance, MeshId, InstanceId, nullptr);
	Scene.GetTransformProxy()->BindTransform(InstanceId, TransformId);
	return InstanceId;
}

void StaticMeshSceneProxy::UpdateInstance(uint InstanceId, uint MeshId)
{
	CommandQueue.emplace_back(UInstance, InstanceId, MeshId, nullptr);
}

void StaticMeshSceneProxy::RemoveInstance(uint InstanceId)
{
	CommandQueue.emplace_back(DInstance, InstanceId, 0, nullptr);
}

void StaticMeshSceneProxy::SetInstanceVisibility(uint InstanceId, bool bVisible)
{
	CommandQueue.emplace_back(UVisibility, InstanceId, bVisible, nullptr);
}

void StaticMeshSceneProxy::UpdateStaticMeshGeometry(uint MeshId, StaticMesh* InMesh)
{
	CommandQueue.emplace_back(UMesh, MeshId, 0, InMesh);
}

void StaticMeshSceneProxy::RemoveStaticMesh(uint MeshId)
{
	CommandQueue.emplace_back(DMesh, MeshId, 0, nullptr);
}


void StaticMeshSceneProxy::UploadDirtyData(Stream& stream)
{
	bFrameUpdated = false;

	for (auto& Command : CommandQueue)
	{
		auto Type = std::get<0>(Command);
		auto Id1 = std::get<1>(Command);
		auto Id2 = std::get<2>(Command);
		auto MeshPtr = std::get<3>(Command);
		switch (Type)
		{
			case CMesh:
			{
				if (!MeshPtr || MeshPtr->IsEmpty())
				{
					LOG_ERROR("Add an empty mesh to scene: {}", MeshPtr->GetName());
					continue;
				}
				bFrameUpdated = true;
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
				StaticMeshData[Id1] = { VBindlessid, TBindlessid, CNBindlessid, MaterialID };
				MeshResources[Id1] = { AccelMesh, VBuffer, TBuffer, CornerlNormalBuffer };
				break;
			}
			case CInstance:
			{
				accel.set_mesh(Id2, *MeshResources[Id1].AccelMesh);
				accel.set_instance_user_id_on_update(Id2, Id1);
				MeshInstances[Id1].insert(Id2);
				break;
			}
			case UVisibility:
			{
				accel.set_visibility_on_update(Id1, Id2);
				break;
			}
			case UMesh:
			{
				if (!MeshPtr || MeshPtr->IsEmpty())
				{
					LOG_ERROR("Update an empty mesh to scene: {}", MeshPtr->GetName());
					continue;
				}
				bFrameUpdated = true;
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

				ASSERT(VBindlessid != ~0u && TBindlessid != ~0u && CNBindlessid != ~0u);
				bindlessArray.emplace_on_update(VBindlessid, VBuffer->view());
				bindlessArray.emplace_on_update(TBindlessid, TBuffer->view());
				bindlessArray.emplace_on_update(CNBindlessid, CornerNormalBuffer->view());

				for (auto Instance : MeshInstances[Id1])
					accel.set_mesh(Instance, *AccelMesh);

				StaticMeshData[Id1] = { VBindlessid, TBindlessid, CNBindlessid, MaterialID};
				MeshResources[Id1] = { AccelMesh, VBuffer, TBuffer, CornerNormalBuffer };
				auto PreVBuffer = MeshResources[Id1].VertexBuffer;
				auto PreTBuffer = MeshResources[Id1].TriangleBuffer;
				auto PreCNBuffer = MeshResources[Id1].CornerNormalBuffer;
				auto PreMesh = MeshResources[Id1].AccelMesh;
				Scene.destroy(PreVBuffer);
				Scene.destroy(PreTBuffer);
				Scene.destroy(PreCNBuffer);
				Scene.destroy(PreMesh);
				break;
			}
			case UInstance: // Not tested
			{
				for (int i = 0; i < MeshIdCounter; i++)
				{
					if (MeshInstances[i].count(Id1))
					{
						MeshInstances[i].erase(Id1);
						break;
					}
				}
				MeshInstances[Id2].insert(Id1);
				accel.set_mesh(Id1, *MeshResources[Id2].AccelMesh);
				accel.set_instance_user_id_on_update(Id1, Id2);
				break;
			}
			case DInstance: // Not tested
			{
				accel.set_visibility_on_update(Id1, false);
				break;
			}
			case DMesh:
			{
				for (auto Instance : MeshInstances[Id1])
					accel.set_visibility_on_update(Instance, false);
				auto PreVBuffer = MeshResources[Id1].VertexBuffer;
				auto PreTBuffer = MeshResources[Id1].TriangleBuffer;
				auto PreCNBuffer = MeshResources[Id1].CornerNormalBuffer;
				auto PreMesh = MeshResources[Id1].AccelMesh;
				Scene.destroy(PreVBuffer);
				Scene.destroy(PreTBuffer);
				Scene.destroy(PreCNBuffer);
				Scene.destroy(PreMesh);
				MeshResources[Id1] = {};
				break;
			}
		}
	}
	CommandQueue.clear();

	if (bFrameUpdated)
		stream << data_buffer.subview(0, StaticMeshData.size()).copy_from(StaticMeshData.data());

	if (accel.dirty())
		stream << accel.build();
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