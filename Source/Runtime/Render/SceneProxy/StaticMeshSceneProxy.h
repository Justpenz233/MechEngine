//
// Created by MarvelLi on 2024/4/4.
//

#pragma once
#include "SceneProxy.h"
#include "Render/Core/VertexData.h"

class StaticMeshComponent;
namespace MechEngine::Rendering
{
struct static_mesh_data
{
	// Vertex buffer id in bindless array
	uint vertex_id = ~0u;

	// Triangle id in bindless array
	uint triangle_id = ~0u;

	// Corner normal id in bindless array
	uint corner_normal_id = ~0u;

	// Material id
	uint material_id = ~0u;
};
}

LUISA_STRUCT(MechEngine::Rendering::static_mesh_data, vertex_id, triangle_id, corner_normal_id, material_id) {};

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

	[[nodiscard]] Var<static_mesh_data> get_static_mesh_data(const UInt& mesh_index) const
	{
		return bindelss_buffer<static_mesh_data>(data_buffer_id)->read(mesh_index);
	}

	[[nodiscard]] Var<Triangle> get_triangle(const UInt& instance_id, const UInt& triangle_index) const
	{
		auto mesh_data = get_static_mesh_data(instance_id);
		return bindlessArray->buffer<Triangle>(mesh_data.triangle_id)->read(triangle_index);
	}

	[[nodiscard]] Var<Vertex> get_vertex(const UInt& instance_id, const UInt& vertex_index) const
	{
		auto mesh_data = get_static_mesh_data(instance_id);
		return bindlessArray->buffer<Vertex>(mesh_data.vertex_id)->read(vertex_index);
	}

	[[nodiscard]] Float3 get_corner_normal(const UInt& instance_id, const UInt& triangle_index, const UInt& corner_id) const
	{
		auto mesh_data = get_static_mesh_data(instance_id);
		return bindlessArray->buffer<float3>(mesh_data.corner_normal_id)->read(triangle_index * 3 + corner_id);
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
	vector<static_mesh_data> StaticMeshDatas;
	BufferView<static_mesh_data> data_buffer;
	uint data_buffer_id; // bindless array id of data buffer, this id will never change

	map<uint, StaticMeshComponent*> TransformMeshMap; // used for mapping transform id to mesh id

	map<StaticMeshComponent*, uint> MeshIndexMap; // used in update mesh
	set<StaticMeshComponent*> NewMeshes;
	set<StaticMeshComponent*> DirtyMeshes;
};

}
