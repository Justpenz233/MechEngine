//
// Created by MarvelLi on 2025/1/6.
//

#include "rasterizer.h"

#include "Render/PipeLine/GpuScene.h"
#include "Render/SceneProxy/CameraSceneProxy.h"
#include "Render/SceneProxy/ShapeSceneProxy.h"
#include "Render/SceneProxy/StaticMeshSceneProxy.h"
#include "Render/SceneProxy/TransformProxy.h"

namespace MechEngine::Rendering
{

UInt rasterizer::get_mesh_id(const UInt& instance_id) const
{
	auto ShapeProxy = scene->GetShapeProxy();
	return ShapeProxy->get_instance_shape(instance_id)->mesh_id;
}

Float4x4 rasterizer::get_instance_transform_mat(const UInt& instance_id) const
{
	return scene->GetTransformProxy()->get_instance_transform_data(instance_id).transform_matrix;
}

ArrayVar<Vertex, 3> rasterizer::get_vertices(const UInt& mesh_id, const UInt& triangle_id) const
{
	auto MeshProxy = scene->GetStaticMeshProxy();
	return MeshProxy->get_vertices(mesh_id, triangle_id);
}
Var<Vertex> rasterizer::get_vertex(const UInt& mesh_id, const UInt& vertex_index) const
{
	return scene->GetStaticMeshProxy()->get_vertex(mesh_id, vertex_index);
}
Var<Triangle> rasterizer::get_triangle(const UInt& mesh_id, const UInt& triangle_index) const
{
	return scene->GetStaticMeshProxy()->get_triangle(mesh_id, triangle_index);
}

Var<view> rasterizer::get_view() const
{
	return scene->GetCameraProxy()->get_main_view();
}

Bool rasterizer::back_face_culling(const ArrayFloat3<3>& vertex_scree_coords)
{
	static Callable func = [](const ArrayFloat3<3>& Vertex) {
		auto normal = cross(Vertex[1] - Vertex[0], Vertex[2] - Vertex[0]);
		return normal.z < 0.f;
	};
	return func(vertex_scree_coords);
}

} // namespace MechEngine::Rendering