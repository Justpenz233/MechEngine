//
// Created by MarvelLi on 2024/3/17.
//

#include "Components/SCAFParametricMeshComponent.h"
#include <bvh/v2/default_builder.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <igl/doublearea.h>

#include "Algorithm/GeometryProcess.h"
#include "igl/exact_geodesic.h"
#include "igl/boundary_loop.h"
#include "igl/flipped_triangles.h"
#include "igl/harmonic.h"
#include "igl/MappingEnergyType.h"
#include "igl/map_vertices_to_circle.h"
#include "igl/topological_hole_fill.h"
#include "igl/triangle/scaf.h"


using Scalar  = double;
using Vec3    = bvh::v2::Vec<Scalar, 3>;
using BBox    = bvh::v2::BBox<Scalar, 3>;
using Tri     = bvh::v2::Tri<Scalar, 3>;
using Node    = bvh::v2::Node<Scalar, 3>;
using Bvh     = bvh::v2::Bvh<Node>;
using Ray     = bvh::v2::Ray<Scalar, 3>;

SCAFParametricMeshComponent::SCAFParametricMeshComponent(const ObjectPtr<StaticMesh>& InDisplayMesh, const ObjectPtr<StaticMesh>& InPMesh)
	: ParametricAlgorithmComponent(InDisplayMesh, InPMesh)
{
	MatrixX3d& V = PMesh->verM;
	MatrixX3i& F = PMesh->triM;

	Vertices = V;
	Indices = F;
	Triangles.resize(F.rows());

	Eigen::MatrixXd bnd_uv, uv_init;

	Eigen::VectorXd M;
	igl::doublearea(V, F, M);
	std::vector<std::vector<int>> all_bnds;
	igl::boundary_loop(F, all_bnds);

	ASSERTMSG(!all_bnds.empty(), "no boundary found, the mesh should have at least one boundary");

	// Heuristic primary boundary choice: longest
	auto primary_bnd = std::max_element(all_bnds.begin(), all_bnds.end(), [](const std::vector<int>& a, const std::vector<int>& b) { return a.size() < b.size(); });

	Eigen::VectorXi bnd = Eigen::Map<Eigen::VectorXi>(primary_bnd->data(), primary_bnd->size());

	igl::map_vertices_to_circle(V, bnd, bnd_uv);
	bnd_uv *= sqrt(M.sum() / (2 * igl::PI));
	if (all_bnds.size() == 1)
	{
		if (bnd.rows() == V.rows()) // case: all vertex on boundary
		{
			uv_init.resize(V.rows(), 2);
			for (int i = 0; i < bnd.rows(); i++)
				uv_init.row(bnd(i)) = bnd_uv.row(i);
		}
		else
		{
			igl::harmonic(V, F, bnd, bnd_uv, 1, uv_init);
			if (igl::flipped_triangles(uv_init, F).size() != 0)
				igl::harmonic(F, bnd, bnd_uv, 1, uv_init); // fallback uniform laplacian
		}
	}
	else
	{
		// if there is a hole, fill it and erase additional vertices.
		all_bnds.erase(primary_bnd);
		Eigen::MatrixXi F_filled;
		igl::topological_hole_fill(F, all_bnds, F_filled);
		igl::harmonic(F_filled, bnd, bnd_uv, 1, uv_init);
		uv_init.conservativeResize(V.rows(), 2);
	}
	igl::triangle::SCAFData scaf_data;

	Eigen::VectorXi b;
	Eigen::MatrixXd bc;
	igl::triangle::scaf_precompute(V, F, uv_init, scaf_data, igl::MappingEnergyType::LOG_ARAP, b, bc, 0);

	// Solve the SCAF
	igl::triangle::scaf_solve(scaf_data, 100);

	// Plot the mesh

	auto UV_Temp = scaf_data.w_uv.topRows(V.rows()).eval();
	UV_Temp.array() -= UV_Temp.minCoeff();
	UV_Temp *= 1. / (UV_Temp.maxCoeff() - UV_Temp.minCoeff());

	UVMesh.resize(UV_Temp.rows(), 3);
	for (int i = 0; i < UV_Temp.rows(); i++)
		UVMesh.row(i) = FVector{ UV_Temp(i, 0), UV_Temp(i, 1), 0 };
	// Set up the BVH tree
	auto Config = bvh::v2::DefaultBuilder<BVHNode>::Config();
	Config.quality = bvh::v2::DefaultBuilder<BVHNode>::Quality::High;
	TArray<BBox> BBoxes(F.rows());
	TArray<Vec3> Centers(F.rows());
	for (int i = 0; i < F.rows(); i++)
	{
		Vec3 V0 = Vec3(UVMesh(F(i, 0), 0), UVMesh(F(i, 0), 1), 0);
		Vec3 V1 = Vec3(UVMesh(F(i, 1), 0), UVMesh(F(i, 1), 1), 0);
		Vec3 V2 = Vec3(UVMesh(F(i, 2), 0), UVMesh(F(i, 2), 1), 0);
		Tri	 T = Tri(V0, V1, V2);
		Triangles[i] = T;
		BBoxes[i] = T.get_bbox();
		Centers[i] = T.get_center();
	}
	BVHUVMesh = bvh::v2::DefaultBuilder<Node>::build(BBoxes, Centers, Config);
}

ParametricAlgorithmComponent::UVMappingSampleResult SCAFParametricMeshComponent::SampleHit(double U, double V) const
{
	/**
	 * Modify UV into a circular parameterization coordinate
	 * Because the initial guess is a circle and then optimize to a better parameterization
	 * Theta = 2 * PI * U [0, 2pi]
	 * R = V  [0, 1]
	 * NewU = R * (cos(Theta) + 1) * 0.5
	 * NewV = R * (sin(Theta) + 1) * 0.5
	 */
	double Theta = MATH_2_PI * U;
	double R = V;
	U = (cos(Theta) * R) * 0.5 + 0.5;
	V = (sin(Theta) * R) * 0.5 + 0.5;


	// Shot a ray from the UV space
	auto ray = Ray {
		Vec3(U, V, 1.), // Ray origin
		Vec3(0., 0., -1.), // Ray direction
		0.,               // Minimum intersection distance
		2.              // Maximum intersection distance
	};

	static constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	static constexpr size_t stack_size = 64;
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
			LOG_WARNING_IF(HitCount > 1,"Multi Hit: {}, not flip free!", HitCount);
			return prim_id != invalid_id;
		});

	if (prim_id == invalid_id)
	{
		return {false};
	}

	auto T1 = MeshData->triM(prim_id, 0);
	auto T2 = MeshData->triM(prim_id, 1);
	auto T3 = MeshData->triM(prim_id, 2);

	auto Result = MeshData->verM.row(T1) * (1. - u - v) + MeshData->verM.row(T2) * u + MeshData->verM.row(T3) * v;

	return {true, static_cast<int>(prim_id), u, v,{Result[0], Result[1], Result[2]}};
}


TArray<FVector> SCAFParametricMeshComponent::GeodicShortestPath(const FVector& Start, const FVector& End) const
{
	auto FindNearestTriangle = [&](const FVector& Pos) {
		double Distance = std::numeric_limits<double>::max();
		int Best = 0;
		for (int i = 0;i < MeshData->triM.rows();i ++)
		{
			FVector V0 = MeshData->verM.row(MeshData->triM(i, 0));
			FVector V1 = MeshData->verM.row(MeshData->triM(i, 1));
			FVector V2 = MeshData->verM.row(MeshData->triM(i, 2));
			FVector Center = (V0 + V1 + V2) / 3.;
			if (double CD = (Center - Pos).norm(); Distance > CD)
			{
				Distance = CD;
				Best = i;
			}
		}
		return Best;
	};
	int StartTriIndex = FindNearestTriangle(Start);
	int EndTriIndex = FindNearestTriangle(End);
	return igl::exact_geodesic_path(MeshData->verM, MeshData->triM, Start, End,
			StartTriIndex, EndTriIndex);;
}
