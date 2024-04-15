//
// Created by MarvelLi on 2024/4/4.
//

#pragma once
#include "SceneProxy.h"
#include "Render/Core/VertexData.h"

class StaticMeshComponent;
namespace MechEngine::Rendering
{
struct staticMeshData
{
	// Vertex buffer id in bindless array
	uint vertex_id = ~0u;

	// Triangle id in bindless array
	uint triangle_id = ~0u;

	// Material id
	uint material_id = ~0u;
};
}

LUISA_STRUCT(MechEngine::Rendering::staticMeshData, vertex_id, triangle_id, material_id) {};

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
class StaticMeshSceneProxy : public SceneProxy
{
public:
	StaticMeshSceneProxy(RayTracingScene& InScene);

	virtual void UploadDirtyData(Stream& stream) override;

	void AddStaticMesh(StaticMeshComponent* InMesh, uint InTransformID);
	void UpdateStaticMesh(StaticMeshComponent* InMesh);

//----------------- GPU CODE -----------------
	Var<staticMeshData> get_static_mesh_data(Expr<uint> mesh_index) const
	{
		return data_buffer->read(mesh_index);
	}

	Var<Triangle> get_triangle(Expr<uint> mesh_index, Expr<uint> triangle_index) const
	{
		auto mesh_data = data_buffer->read(mesh_index);
		return bindlessArray->buffer<Triangle>(mesh_data.triangle_id)->read(triangle_index);
	}

	Var<Vertex> get_vertex(Expr<uint> mesh_index, Expr<uint> vertex_index) const
	{
		auto mesh_data = data_buffer->read(mesh_index);
		return bindlessArray->buffer<Vertex>(mesh_data.vertex_id)->read(vertex_index);
	}

public://----------------- CPU CODE -----------------
	uint TransformIdToMeshId(uint InTransformID)
	{
		if (!TransformMeshMap.count(InTransformID)) return ~0u;
		return MeshIndexMap[TransformMeshMap[InTransformID]];
	}

protected:

	//! We are using accel index as id for mesh, not the data array index
	static constexpr auto instance_max_number = 65536u;
	vector<staticMeshData> StaticMeshDatas;
	BufferView<staticMeshData> data_buffer;

	map<uint, StaticMeshComponent*> TransformMeshMap; // used for mapping transform id to mesh id

	map<StaticMeshComponent*, uint> MeshIndexMap; // used in update mesh
	set<StaticMeshComponent*> NewMeshes;
	set<StaticMeshComponent*> DirtyMeshes;
};

}
