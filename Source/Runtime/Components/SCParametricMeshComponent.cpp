//
// Created by MarvelLi on 2024/6/6.
//

#include "SCParametricMeshComponent.h"

#include "Algorithm/GeometryProcess.h"
#include "igl/barycenter.h"
#include "igl/barycentric_interpolation.h"
#include "igl/cotmatrix.h"
#include "igl/grad.h"
#include "igl/point_mesh_squared_distance.h"

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

SCParametricMeshComponent::SCParametricMeshComponent(ObjectPtr<StaticMesh> InitMesh, int Iteration)
{
	Eigen::MatrixX3d V,U;
	Eigen::MatrixXi F;
	Eigen::SparseMatrix<double> L;

	V = InitMesh->GetVertices();
	F = InitMesh->GetTriangles();
	Vertices = V;
	Indices = F;
	// Compute Laplace-Beltrami operator: #V by #V
	igl::cotmatrix(V,F,L);

	// Alternative construction of same Laplacian
	Eigen::SparseMatrix<double> G,K;
	// Gradient/Divergence
	igl::grad(V, F,G);
	// Diagonal per-triangle "mass matrix"
	VectorXd dblA;
	igl::doublearea(V,F,dblA);
	// Place areas along diagonal #dim times
	const auto & T = 1.*(dblA.replicate(3,1)*0.5).asDiagonal();
	// Laplacian K built as discrete divergence of gradient or equivalently
	// discrete Dirichelet energy Hessian
	K = -G.transpose() * T * G;
	U = V;
	for(int i = 0; i < Iteration; i ++){
		// Recompute just mass matrix on each step
		Eigen::SparseMatrix<double> M;
		igl::massmatrix(U,F,igl::MASSMATRIX_TYPE_BARYCENTRIC,M);
		// Solve (M-delta*L) U = M*U
		const auto & S = (M - 0.001*L);
		Eigen::SimplicialLLT<Eigen::SparseMatrix<double > > solver(S);
		assert(solver.info() == Eigen::Success);
		U = solver.solve(M*U).eval();
		// Compute centroid and subtract (also important for numerics)
		VectorXd dblA;
		igl::doublearea(U,F,dblA);
		double area = 0.5*dblA.sum();
		MatrixXd BC;
		igl::barycenter(U,F,BC);
		RowVector3d centroid(0,0,0);
		for(int i = 0;i<BC.rows();i++)
		{
			centroid += 0.5*dblA(i)/area*BC.row(i);
		}
		U.rowwise() -= centroid;
		// Normalize to unit surface area (important for numerics)
		U.array() /= sqrt(area);
	}
	MeshData = std::move(InitMesh);

	// Build BVH for sample
	auto Config = bvh::v2::DefaultBuilder<BVHNode>::Config();
	Config.quality = bvh::v2::DefaultBuilder<BVHNode>::Quality::High;
	Triangles.resize(F.rows());
	// Normalize U To box[-1, 1]
	double Max = U.maxCoeff(); double Min = U.minCoeff();
	U = (U.array() - Min) / (Max - Min) * 2. - 1.;

	TArray<BBox> BBoxes(F.rows());
	TArray<Vec3> Centers(F.rows());
	auto ToVec3 = [](const FVector& T){ return Vec3(T.x(), T.y(), T.z());};
	for (int i = 0;i < F.rows();i ++)
	{
		Vec3 V0 = ToVec3(U.row(F(i, 0)));
		Vec3 V1 = ToVec3(U.row(F(i, 1)));
		Vec3 V2 = ToVec3(U.row(F(i, 2)));
		Tri T = Tri(V0, V1, V2); 		Triangles[i] = T;
		BBoxes[i] = T.get_bbox();
		Centers[i] = T.get_center();
	}
	BVHUVMesh = bvh::v2::DefaultBuilder<Node>::build(BBoxes, Centers, Config);
}
FVector SCParametricMeshComponent::Sample(double U, double V) const
{
	NormlizeUV(U, V);
	auto Hit = SampleHit(U, V);
	if(Hit.Valid)
		return SampleHit(U, V).Position;
	else
		return FVector::Zero();

}
FVector SCParametricMeshComponent::SampleNormal(double u, double v) const
{
	auto Hit = SampleHit(u, v);

	// calc vertex normal by barycentric interpolation
	auto T1 = MeshData->triM(Hit.TriangleIndex, 0);
	auto T2 = MeshData->triM(Hit.TriangleIndex, 1);
	auto T3 = MeshData->triM(Hit.TriangleIndex, 2);
	auto N1 = MeshData->VertexNormal.row(T1);
	auto N2 = MeshData->VertexNormal.row(T2);
	auto N3 = MeshData->VertexNormal.row(T3);
	return N1 * (1. - Hit.u - Hit.v) + N2 * Hit.u + N3 * Hit.v;
}
bool SCParametricMeshComponent::ValidUV(double U, double V) const
{
	return true;
}
TArray<FVector> SCParametricMeshComponent::GeodicShortestPath(const FVector& Start, const FVector& End) const
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
FVector2 SCParametricMeshComponent::Projection(const FVector& Point) const
{
	return Algorithm::GeometryProcess::Projection(Point, [&](const FVector2& UV) {
	return Sample(UV.x(), UV.y());
});
}
SCParametricMeshComponent::UVMappingSampleResult SCParametricMeshComponent::SampleHit(double U, double V) const
{
	//https://mathworld.wolfram.com/SphericalCoordinates.html
	V = std::clamp(V, 0., 1.);
	V *= M_PI;

	U = std::fmod(U, 1.);
	if(U < 0.) U += 1.;
	U *= 2. * M_PI;

	auto ray = Ray {
		Vec3(0., 0., 0.), // Ray direction
		normalize(Vec3(sin(V) * cos(U), sin(V) * sin(U), cos(V))), // Ray origin
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
			LOG_WARNING_IF(HitCount > 1,"Multi Hit: {}, not flip free!", HitCount);
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