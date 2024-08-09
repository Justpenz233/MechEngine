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
	double w = igl::fast_winding_number(fwn_bvh, 2, Point.transpose().template cast<float>());
	double s = 1. - 2. * std::abs(w);
	return s * Sign < 0.;
}

double OrientedSurfaceComponent::Distance(const FVector& Point) const
{
	int i; Eigen::RowVector3d c;
	return sqrt(AABB.squared_distance(V, F, Point.transpose(), i, c));
}

void OrientedSurfaceComponent::Build(const ObjectPtr<StaticMesh>& OrientedMesh, bool bUseWindingNumber, bool bInverse)
{
	bWindingNumber = bUseWindingNumber;
	Sign = bInverse ? -1. : 1.;
	V = OrientedMesh->GetVertices();
	F = OrientedMesh->GetTriangles();
	AABB.init(V, F);
	if (bUseWindingNumber)
	{
		// Use winding number from libigl
		int order = 2;
		igl::fast_winding_number(OrientedMesh->GetVertices(), OrientedMesh->GetTriangles(), order, fwn_bvh);
	}
	else
	{
		ASSERTMSG(false, "Not implemented");
	}
}