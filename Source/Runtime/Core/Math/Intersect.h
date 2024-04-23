//
// Created by MarvelLi on 2024/2/28.
//

#pragma once
#include "CoreMinimal.h"
#include "Mesh/StaticMesh.h"
#include "Box.h"

namespace  MechEngine::Math
{
/// \brief Calculate the intersection point of a line and a plane
/// \param P0 Point on the line begin
/// \param P1 Point on the line end
/// \param PlanePoint Point on the plane
/// \param PlaneNormal Plane normal
/// \return t as math formula: P = P0 + t(P1 - P0) which intersect the plane
double SegmentIntersectPlane(const FVector& P0, const FVector& P1, const FVector& PlanePoint, const FVector& PlaneNormal);

/// \brief Check if two segments intersect in 2D
/// \param A0 Coordinate of the first point of the first segment
/// \param A1 Coordinate of the second point of the first segment
/// \param B0 Coordinate of the first point of the second segment
/// \param B1 Coordinate of the second point of the second segment
/// \return true if intersect
bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1);


/// \brief Check if two segments intersect in 2D
/// \param A0 Coordinate of the first point of the first segment
/// \param A1 Coordinate of the second point of the first segment
/// \param B0 Coordinate of the first point of the second segment
/// \param B1 Coordinate of the second point of the second segment
/// \return true if intersect, u and t formular: P = A0 + u(A1 - A0), P = B0 + t(B1 - B0)
bool SegmentIntersectSegment2D(const FVector2& A0, const FVector2& A1, const FVector2& B0, const FVector2& B1, double& u, double &t);

/**
 * \brief Check if two mesh intersect
 * \return true if intersect
 */
bool MeshIntersectMesh(const ObjectPtr<class StaticMesh>& MeshA, const ObjectPtr<StaticMesh>& MeshB);

/**
 * \brief Check if a mesh intersect with a box
 * \return true if intersect
 */
template <class T>
bool MeshIntersectBox(const ObjectPtr<StaticMesh>& Mesh, const TBox<T>& Box);

};



template <class T>
bool MechEngine::Math::MeshIntersectBox(const ObjectPtr<StaticMesh>& Mesh, const TBox<T>& Box)
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