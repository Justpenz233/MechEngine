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

LUISA_STRUCT(MechEngine::Rendering::static_mesh_data, vertex_buffer_id, triangle_buffer_id, corner_normal_buffer_id, material_id) {};

namespace MechEngine::Rendering
{
using namespace luisa::compute;
using luisa::uint;

class ENGINE_API StaticMeshSceneProxy : public SceneProxy
{
public:
	explicit StaticMeshSceneProxy(RayTracingScene& InScene);

	virtual bool IsDirty() override;

	virtual void UploadDirtyData(Stream& stream) override;

	/**
	* Upload a new mesh to the scene
	* @param InMesh Static mesh to upload
	* @return mesh id of the mesh
	*/
	uint AddStaticMesh(StaticMesh* InMesh);

	/**
	* Add a new instance of the mesh to the scene
	* @param MeshId Mesh id of the mesh
	* @param TransformId Transform id of the instance
	* @return instance id of the new instance
	*/
	[[nodiscard]] uint AddInstance(uint MeshId, uint TransformId);

	/**
	 * Bind instance to a new mesh
	 * @param InstanceId Instance id
	 * @param MeshId Mesh id
	 */
	void UpdateInstance(uint InstanceId, uint MeshId);

	/**
	* Remove the instance from the scene
	* @param InstanceId Instance id
	*/
	void RemoveInstance(uint InstanceId);

	/**
	* Set the visibility of the instance
	* @param InstanceId Instance id
	* @param bVisible Visibility
	*/
	void SetInstanceVisibility(uint InstanceId, bool bVisible);

	/**
	* Update the mesh geometry in the scene
	*/
	void UpdateStaticMeshGeometry(uint MeshId, StaticMesh* InMesh);

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

	// Create default meshes and upload to GPU
	void CreateDefaultMeshes();

	/**
	 * Get the flatten mesh data from mesh data, used for uploading mesh data to GPU
	 * @param MeshData Mesh data to flatten
	 * @return Flattened mesh data, vertices, triangles, corner normals in GPU format
	 */
	static std::tuple<vector<Vertex>, vector<Triangle>, vector<float3>> GetFlattenMeshData(StaticMesh* MeshData);

protected:
	bool bFrameUpdated = false;

	//! We are using accel index as id for mesh, not the data array index
	static constexpr auto instance_max_number = 65536u;
	BufferView<static_mesh_data> data_buffer;
	vector<static_mesh_data> StaticMeshData;

	vector<StaticMeshResource> MeshResources;

	// MeshId corresponding instances' id
	vector<set<uint>> MeshInstances;

	uint data_buffer_id; // bindless array id of data buffer, this id will never change

	// ID is index in the MeshResources/StaticMeshData
	uint MeshIdCounter = 0;

	enum CommandType
	{
		CMesh,
		CInstance,
		UVisibility,
		UMesh,
		UInstance,
		DInstance,
		DMesh,
	};
	vector<std::tuple<CommandType, uint, uint, StaticMesh*>> CommandQueue;
};

}
