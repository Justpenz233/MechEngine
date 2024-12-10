//
// Created by MarvelLi on 2024/4/4.
//

#pragma once
#include "SceneProxy.h"
#include "Render/Core/VertexData.h"

class StaticMesh;

namespace MechEngine::Rendering
{
struct static_mesh_data
{
	// Vertex buffer id in bindless array
	uint vertex_buffer_id = ~0u;

	// Triangle id in bindless array
	uint triangle_buffer_id = ~0u;

	// Corner normal id in bindless array
	uint corner_normal_buffer_id = ~0u;

	// Material id
	uint material_id = ~0u;

	// Mesh tag
	uint mesh_tag = 0u;

	auto valid() const
	{
		return vertex_buffer_id != ~0u && triangle_buffer_id != ~0u && corner_normal_buffer_id != ~0u && material_id != ~0u;
	}
};

struct StaticMeshResource
{
	// CPU only
	Mesh* AccelMesh = nullptr;
	Buffer<Vertex>* VertexBuffer = nullptr;
	Buffer<Triangle>* TriangleBuffer = nullptr;
	Buffer<float3>* CornerNormalBuffer = nullptr;
};
}

LUISA_STRUCT(MechEngine::Rendering::static_mesh_data, vertex_buffer_id, triangle_buffer_id, corner_normal_buffer_id, material_id, mesh_tag)
{
	auto valid()
	{
		return vertex_buffer_id != ~0u & triangle_buffer_id != ~0u & corner_normal_buffer_id != ~0u;
	}
};

namespace MechEngine::Rendering
{
using namespace luisa::compute;
using luisa::uint;

class ENGINE_API StaticMeshSceneProxy : public SceneProxy
{
public:
	explicit StaticMeshSceneProxy(GpuScene& InScene);

	virtual bool IsDirty() override;

	virtual void UploadDirtyData(Stream& stream) override;

	/**
	* Upload a new mesh to the scene
	* @param InMesh Static mesh to upload
	* @return mesh id of the mesh
	*/
	uint AddStaticMesh(StaticMesh* InMesh);

	/**
	* Update the mesh geometry corresponding to the MeshID in the GPU
	* @param MeshId Mesh id
	* @param InMesh Mesh data
	*/
	void UpdateStaticMeshGeometry(uint MeshId, StaticMesh* InMesh);

	/**
	 * Bind the mesh to the instance
	 * @param InstanceID Instance id
	 * @param MeshID Mesh id
	 */
	void BindInstance(uint MeshID, uint InstanceID);

	/**
	 * Remove the mesh from the scene
	 * @param MeshId Mesh id
	 */
	void RemoveStaticMesh(uint MeshId);


	/***********************************************************************************************
	 * 								            GPU CODE						                   *
	 ***********************************************************************************************/
	[[nodiscard]] Var<static_mesh_data> get_static_mesh_data(const UInt& mesh_id) const
	{
		return bindelss_buffer<static_mesh_data>(data_buffer_id)->read(mesh_id);
	}

	UInt get_mesh_tag(const UInt& mesh_id) const
	{
		return get_static_mesh_data(mesh_id)->mesh_tag;
	}

	[[nodiscard]] Var<Triangle> get_triangle(const UInt& mesh_id, const UInt& triangle_index) const
	{
		auto mesh_data = get_static_mesh_data(mesh_id);
		return bindelss_buffer<Triangle>(mesh_data.triangle_buffer_id)->read(triangle_index);
	}

	[[nodiscard]] Var<Vertex> get_vertex(const UInt& mesh_id, const UInt& vertex_index) const
	{
		auto mesh_data = get_static_mesh_data(mesh_id);
		return bindelss_buffer<Vertex>(mesh_data.vertex_buffer_id)->read(vertex_index);
	}

	[[nodiscard]] ArrayVar<Vertex, 3> get_vertices(const UInt& mesh_id, const UInt& triangle_index) const
	{
		auto mesh_data = get_static_mesh_data(mesh_id);
		auto triangle = bindelss_buffer<Triangle>(mesh_data.triangle_buffer_id)->read(triangle_index);
		return{
			bindelss_buffer<Vertex>(mesh_data.vertex_buffer_id)->read(triangle.i0),
			bindelss_buffer<Vertex>(mesh_data.vertex_buffer_id)->read(triangle.i1),
			bindelss_buffer<Vertex>(mesh_data.vertex_buffer_id)->read(triangle.i2)};
	}

	[[nodiscard]] Float3 get_corner_normal(const UInt& mesh_id, const UInt& triangle_index, const UInt& corner_id) const
	{
		auto mesh_data = get_static_mesh_data(mesh_id);
		return bindelss_buffer<float3>(mesh_data.corner_normal_buffer_id)->read(triangle_index * 3 + corner_id);
	}

	[[nodiscard]] ArrayVar<Float3, 3> get_corner_normals(const UInt& mesh_id, const UInt& triangle_index) const
	{
		auto mesh_data = get_static_mesh_data(mesh_id);
		return{
			bindelss_buffer<float3>(mesh_data.corner_normal_buffer_id)->read(triangle_index * 3 + 0),
			bindelss_buffer<float3>(mesh_data.corner_normal_buffer_id)->read(triangle_index * 3 + 1),
			bindelss_buffer<float3>(mesh_data.corner_normal_buffer_id)->read(triangle_index * 3 + 2)};
	}

protected:

	/**
	 * Get the flatten mesh data from mesh data, used for uploading mesh data to GPU
	 * @param MeshData Mesh data to flatten
	 * @return Flattened mesh data, vertices, triangles, corner normals in GPU format
	 */
	static std::tuple<vector<Vertex>, vector<Triangle>, vector<float3>> GetFlattenMeshData(StaticMesh* MeshData);

protected:
	bool bFrameUpdated = false;

	BufferView<static_mesh_data> data_buffer;
	vector<static_mesh_data> StaticMeshData;

	vector<StaticMeshResource> MeshResources;

	// MeshId corresponding instances' id
	vector<set<uint>> MeshInstances;

	uint data_buffer_id; // bindless array id of data buffer, this id will never change

	// ID is index in the MeshResources/StaticMeshData
	uint MeshIdCounter = 0;

	Mesh* NullMesh = nullptr;

	enum CommandType
	{
		Create,
		Update,
		Delete,
		Bind
	};
	vector<std::tuple<CommandType, uint, uint, StaticMesh*>> CommandQueue;
};

}
