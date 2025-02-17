//
// Created by MarvelLi on 2024/6/26.
//

#include "BCParametricMeshComponent.h"
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Surface_mesh/Surface_mesh.h>
#include <CGAL/Surface_mesh_parameterization/Square_border_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/Discrete_conformal_map_parameterizer_3.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Surface_mesh_parameterization/Discrete_authalic_parameterizer_3.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/boost/graph/Seam_mesh.h>
#include <CGAL/Surface_mesh_parameterization/IO/File_off.h>
#include <CGAL/Surface_mesh_parameterization/parameterize.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/Polygon_mesh_processing/measure.h>

#include <bvh/v2/default_builder.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>

using Scalar  = double;
using Vec3    = bvh::v2::Vec<Scalar, 3>;
using BBox    = bvh::v2::BBox<Scalar, 3>;
using Tri     = bvh::v2::Tri<Scalar, 3>;
using Node    = bvh::v2::Node<Scalar, 3>;
using Bvh     = bvh::v2::Bvh<Node>;
using Ray     = bvh::v2::Ray<Scalar, 3>;


typedef CGAL::Simple_cartesian<double>          Kernel;
typedef Kernel::Point_2                         Point_2;
typedef Kernel::Point_3                         Point_3;
typedef CGAL::Surface_mesh<Kernel::Point_3>     SurfaceMesh;
typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor     vertex_descriptor;
typedef boost::graph_traits<SurfaceMesh>::halfedge_descriptor   halfedge_descriptor;

CGAL::Surface_mesh<Kernel::Point_3> ToCGALSurfaceMesh(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F)
{
	SurfaceMesh sm;
	std::vector<vertex_descriptor> vertices;
	for (int i = 0; i < V.rows(); i++)
	{
		vertices.push_back(sm.add_vertex(Point_3(V(i, 0), V(i, 1), V(i, 2))));
	}
	for (int i = 0; i < F.rows(); i++)
	{
		sm.add_face(vertices[F(i, 0)], vertices[F(i, 1)], vertices[F(i, 2)]);
	}
	return sm;
}


BCParametricMeshComponent::BCParametricMeshComponent(ObjectPtr<StaticMesh> InDisplayMesh, ObjectPtr<StaticMesh> InPMesh)
	: ParametricAlgorithmComponent(InDisplayMesh, InPMesh)
{
	Eigen::MatrixX3d			V;
	Eigen::MatrixXi				F;
	Eigen::SparseMatrix<double> L;

	V = PMesh->GetVertices();
	F = PMesh->GetTriangles();
	Vertices = V;
	Indices = F;

	SurfaceMesh sm = ToCGALSurfaceMesh(Vertices, Indices);
	// a halfedge on the border
	halfedge_descriptor bhd = CGAL::Polygon_mesh_processing::longest_border(sm).first;
	// The UV property map that holds the parameterized values
	typedef SurfaceMesh::Property_map<vertex_descriptor, Point_2>														   UV_pmap;
	UV_pmap																												   uv_map = sm.add_property_map<vertex_descriptor, Point_2>("h:uv").first;
	typedef CGAL::Surface_mesh_parameterization::Square_border_arc_length_parameterizer_3<SurfaceMesh>					   Border_parameterizer;
	typedef CGAL::Surface_mesh_parameterization::Discrete_conformal_map_parameterizer_3<SurfaceMesh, Border_parameterizer> Parameterizer;
	CGAL::Surface_mesh_parameterization::Error_code																		   err = CGAL::Surface_mesh_parameterization::parameterize(sm, Parameterizer(), bhd, uv_map);
	ASSERTMSG(err == CGAL::Surface_mesh_parameterization::OK, "Parameterization failed");

	UVMesh.resize(num_vertices(sm), 3);
	for (auto v : vertices(sm))
	{
		UVMesh(v.idx(), 0) = uv_map[v].x();
		UVMesh(v.idx(), 1) = uv_map[v].y();
		UVMesh(v.idx(), 2) = 0;

		PMesh->SetUV(v.idx(), FVector2(uv_map[v].x(), uv_map[v].y()));
	}
	AABB.init(PMesh->GetVertices(), PMesh->GetTriangles());

	auto Config = bvh::v2::DefaultBuilder<BVHNode>::Config();
	Config.quality = bvh::v2::DefaultBuilder<BVHNode>::Quality::High;
	Triangles.resize(F.rows());

	TArray<BBox> BBoxes(F.rows());
	TArray<Vec3> Centers(F.rows());
	auto		 ToVec3 = [](const FVector& T) { return Vec3(T.x(), T.y(), T.z()); };
	for (int i = 0; i < F.rows(); i++)
	{
		Vec3 V0 = ToVec3(UVMesh.row(F(i, 0)));
		Vec3 V1 = ToVec3(UVMesh.row(F(i, 1)));
		Vec3 V2 = ToVec3(UVMesh.row(F(i, 2)));
		Tri	 T = Tri(V0, V1, V2);
		Triangles[i] = T;
		BBoxes[i] = T.get_bbox();
		Centers[i] = T.get_center();
	}
	BVHUVMesh = bvh::v2::DefaultBuilder<Node>::build(BBoxes, Centers, Config);
	AABB.init(PMesh->GetVertices(), PMesh->GetTriangles());
}

ObjectPtr<StaticMesh> BCParametricMeshComponent::GetUVMesh() const
{
	auto Result = NewObject<StaticMesh>(UVMesh, Indices);
	Result->ReverseNormal();
	return Result;
}

BCParametricMeshComponent::UVMappingSampleResult BCParametricMeshComponent::SampleHit(double U, double V) const
{
	NormlizeUV(U, V);
	auto ray = Ray {
		Vec3(U, V, 1.), // Ray direction
		Vec3(0, 0, -1), // Ray origin
		0., // Minimum distance
		3.
	};

	static constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	static constexpr size_t stack_size = 512;
	static constexpr bool use_robust_traversal = false;

	auto prim_id = invalid_id;
	double u, v;

	// Traverse the BVH and get the u, v coordinates of the closest intersection.
	bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	BVHUVMesh.intersect<false, use_robust_traversal>(ray, BVHUVMesh.get_root().index, stack,
		[&] (size_t begin, size_t end) {
			int HitCount = 0;
			for (size_t i = begin; i < end; ++i) {
				size_t TriangleID = BVHUVMesh.prim_ids[i];
				if (auto hit = Triangles[TriangleID].intersect(ray)) {
					prim_id = TriangleID;
					HitCount ++;
					std::tie(u, v) = *hit;
				}
			}
			// LOG_WARNING_IF(HitCount > 1,"Multi Hit: {}, not flip free!", HitCount);
			return prim_id != invalid_id;
		});

	if (prim_id == invalid_id)
	{
		return {false};
	}

	auto T1 = Indices(prim_id, 0);
	auto T2 = Indices(prim_id, 1);
	auto T3 = Indices(prim_id, 2);

	auto Result = Vertices.row(T1) * (1. - u - v) + Vertices.row(T2) * u + Vertices.row(T3) * v;

	return {true, static_cast<int>(prim_id), u, v,{Result[0], Result[1], Result[2]}};
}