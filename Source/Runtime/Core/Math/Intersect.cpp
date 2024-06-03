//
// Created by MarvelLi on 2024/2/28.
//

#include "Intersect.h"
#include "CoreMinimal.h"
#include "Mesh/StaticMesh.h"
#include "igl/copyleft/cgal/intersect_other.h"

namespace MechEngine::Math
{

double SegmentIntersectPlane(const FVector& P0, const FVector& P1, const FVector& PlanePoint, const FVector& PlaneNormal)
{
	return PlaneNormal.dot(PlanePoint - P0) / PlaneNormal.dot(P1 - P0);
}

bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1)
{
	double s1_x, s1_y, s2_x, s2_y;
	s1_x = A1.x() - A0.x();
	s1_y = A1.y() - A0.y();
	s2_x = B1.x() - B0.x();
	s2_y = B1.y() - B0.y();

	double s, t;
	s = (-s1_y * (A0.x() - B0.x()) + s1_x * (A0.y() - B0.y())) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (A0.y() - B0.y()) - s2_y * (A0.x() - B0.x())) / (-s2_x * s1_y + s1_x * s2_y);

	return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
}

bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1, double& u, double& t)
{
	double s1_x, s1_y, s2_x, s2_y;
	s1_x = A1.x() - A0.x();
	s1_y = A1.y() - A0.y();
	s2_x = B1.x() - B0.x();
	s2_y = B1.y() - B0.y();

	u = (-s1_y * (A0.x() - B0.x()) + s1_x * (A0.y() - B0.y())) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (A0.y() - B0.y()) - s2_y * (A0.x() - B0.x())) / (-s2_x * s1_y + s1_x * s2_y);

	return (u >= 0 && u <= 1 && t >= 0 && t <= 1);
}

std::pair<bool, MatrixXi> MeshIntersectMesh(const ObjectPtr<::StaticMesh>& MeshA, const ObjectPtr<::StaticMesh>& MeshB, bool bFirstOnly)
{
	MatrixXi Result;
	igl::copyleft::cgal::intersect_other(MeshA->verM, MeshA->triM, MeshB->verM, MeshB->triM, bFirstOnly, Result);
	return {Result.rows() > 0, Result};
}

#include <igl/tri_tri_intersect.h>
std::tuple<bool, FVector, FVector> TriangleIntersectTriangle(const FVector& A0, const FVector& A1, const FVector& A2, const FVector& B0, const FVector& B1, const FVector& B2)
{
	bool       coplanar;
	RowVector3d    source, target;
	const bool Result = igl::tri_tri_intersection_test_3d(RowVector3d(A0), RowVector3d(A1), RowVector3d(A2)
		, RowVector3d(B0),RowVector3d(B1), RowVector3d(B2)
		, coplanar, source, target);
	return std::make_tuple(Result, FVector(source), FVector(target));
}

};