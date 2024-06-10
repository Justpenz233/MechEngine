//
// Created by MarvelLi on 2024/2/28.
//

#pragma once
#include "CoreMinimal.h"
#include "Mesh/StaticMesh.h"
#include "Box.h"

namespace  MechEngine::Math
{

/***
 * Calculate the intersection point of a line and a plane
 * @param P0 Point on the line begin
 * @param P1 Point on the line end
 * @param PlanePoint Point on the plane
 * @param PlaneNormal Plane normal
 * @return t as math formula: P = P0 + t(P1 - P0) which intersect the plane
 */
ENGINE_API double SegmentIntersectPlane(const FVector& P0, const FVector& P1, const FVector& PlanePoint, const FVector& PlaneNormal);


/***
 * @brief Check if two segments intersect in 2D
 * @param A0 Coordinate of the first point of the first segment
 * @param A1 Coordinate of the second point of the first segment
 * @param B0 Coordinate of the first point of the second segment
 * @param B1 Coordinate of the second point of the second segment
 * \return true if intersect
 */
ENGINE_API bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1);


/**
 * \brief Check if two segments intersect in 2D
 * \param A0 Coordinate of the first point of the first segment
 * \param A1 Coordinate of the second point of the first segment
 * \param B0 Coordinate of the first point of the second segment
 * \param B1 Coordinate of the second point of the second segment
 * \param u The parameter of the intersection in first segment
 * \param t The parameter of the intersection in second segment
 * \return true if intersect true if intersect, u and t formular: P = A0 + u(A1 - A0), P = B0 + t(B1 - B0)
 */
ENGINE_API bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1, double& u, double &t);

/**
 * Check if two mesh intersect
 * @param MeshA The first mesh
 * @param MeshB The second mesh
 * @param bFirstOnly If true, only return the first intersection
 * @warning the intersection pairs will only have one if bFirstOnly is true
 * @warning the intersection pairs is not strictly accurate, it may have false positive(so strange), should use TriangleIntersectTriangle to check the intersection
 * @return true if intersect, and have intersection pairs of triangle index
 */
ENGINE_API std::pair<bool, MatrixXi>
MeshIntersectMesh(const ObjectPtr<StaticMesh>& MeshA, const ObjectPtr<StaticMesh>& MeshB, bool bFirstOnly = true);

/**
 * Check if a triangle intersect with a triangle
 * @param A0 Coordinate of the first point of the first triangle
 * @param A1 Coordinate of the second point of the first triangle
 * @param A2 Coordinate of the third point of the first triangle
 * @param B0 Coordinate of the first point of the second triangle
 * @param B1 Coordinate of the second point of the second triangle
 * @param B2 Coordinate of the third point of the second triangle
 * @warning This function not perform as result from MeshIntersectMesh, which means the intersection result may not be true for testing the paris from MeshIntersectMesh
 * @return true if intersect, and the segment of intersection(valid only if intersect)
 */
ENGINE_API std::tuple<bool, FVector, FVector>
TriangleIntersectTriangle(const FVector& A0, const FVector& A1, const FVector& A2, const FVector& B0, const FVector& B1, const FVector& B2);

/**
 * Check if a mesh intersect with a box
 * @return true if intersect
 */
template <class T>
inline bool MeshIntersectBox(const ObjectPtr<StaticMesh>& Mesh, const TBox<T>& Box)
{
	FBox MeshBox = Mesh->GetBoundingBox();
	if (!MeshBox.Intersect(Box))
		return false;

	for(FVector Vertex : Mesh->verM.rowwise())
	{
		if(Box.Contain(Vertex))
			return true;
	}

	return false;
}

};



