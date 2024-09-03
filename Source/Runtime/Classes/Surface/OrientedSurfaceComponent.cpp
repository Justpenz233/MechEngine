//
// Created by marvel on 9/8/24.
//

#include "OrientedSurfaceComponent.h"

#include "igl/per_edge_normals.h"
#include "igl/per_face_normals.h"
#include "igl/per_vertex_normals.h"
#include "igl/pseudonormal_test.h"

bool OrientedSurfaceComponent::Inside(const FVector& Point) const
{
	// Use winding number from libigl
	if(bWindingNumber)
	{
		double w = igl::fast_winding_number(fwn_bvh, 2, Point.transpose().template cast<float>());
		double s = 1. - 2. * std::abs(w);
		return s * Sign < 0.;
	}
	else
		return -sdf->contain(Point.cast<float>());
}

double OrientedSurfaceComponent::Distance(const FVector& Point) const
{
	if(bWindingNumber)
	{
		int i; Eigen::RowVector3d c;
		return sqrt(AABB.squared_distance(V, F, Point.transpose(), i, c));
	}
	else
	{
		return abs(sdf->operator()(Point.cast<float>().transpose(), false, 1)[0]);
	}
}

void OrientedSurfaceComponent::Build(const ObjectPtr<StaticMesh>& OrientedMesh, bool bUseWindingNumber, bool bInverse)
{
	bWindingNumber = bUseWindingNumber;
	Sign = bInverse ? -1. : 1.;
	if (bUseWindingNumber)
	{
		V = OrientedMesh->GetVertices();
		F = OrientedMesh->GetTriangles();
		AABB.init(V, F);
		// Use winding number from libigl
		int order = 2;
		igl::fast_winding_number(OrientedMesh->GetVertices(), OrientedMesh->GetTriangles(), order, fwn_bvh);
	}
	else
	{
		sdf = MakeUnique<sdf::SDF>(OrientedMesh->GetVertices().cast<float>(), OrientedMesh->GetTriangles().cast<uint32_t>(), false, true);
	}
}