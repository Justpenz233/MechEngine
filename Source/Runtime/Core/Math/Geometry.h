//
// Created by MarvelLi on 2024/4/22.
//

#pragma once

#include "MathType.h"
#include "PointerTypes.h"
#include "Misc/Platform.h"
class StaticMesh;
namespace MechEngine::Math
{

/**
 * Compute the distance between a point and a segment, along with the closest point on the segment.
 * @param P The point
 * @param A0 Start point of the segment
 * @param A1 Start point of the segment
 * @return The distance between the point and the segment, and the closest point on the segment
 */
ENGINE_API std::tuple<double, FVector> PointSegmentDistance(
		const FVector& P,
		const FVector& A0, const FVector& A1);

/**
 * Calculate the distance between two segments, along with the closest points on each segment.
 * @param A0 Start point of segment A
 * @param A1 End point of segment A
 * @param B0 Start point of segment B
 * @param B1 End point of segment B
 * @return The distance between two segments, the closest point on each segment, and whether the two segments are parallel
 */
ENGINE_API std::tuple<double, FVector, FVector, bool> SegmentSegmentDistance(
		const FVector& A0, const FVector& A1,
		const FVector& B0, const FVector& B1);


/**
 * Calculate the convex hull of a mesh.
 * @param Mesh The mesh to calculate the convex hull of
 * @return The convex hull of the mesh
 */
ENGINE_API ObjectPtr<StaticMesh> ConvexHull(const ObjectPtr<StaticMesh>& Mesh);
}