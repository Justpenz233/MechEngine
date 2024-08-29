//
// Created by marvel on 9/8/24.
//

#pragma once
#include "Components/ActorComponent.h"
#include "Mesh/StaticMesh.h"
#include "igl/AABB.h"
#include "igl/fast_winding_number.h"

/**
 * OrientedSurfaceComponent is a component that represents an oriented surface
 * Provides functions to check inside, outside, distance, and signed distance to the surface
 */
MCLASS(OrientedSurfaceComponent)
class ENGINE_API OrientedSurfaceComponent : public ActorComponent
{
	REFLECTION_BODY(OrientedSurfaceComponent)

public:
	/**
	 * Constructor by a given oriented mesh
	 * @param OrientedMesh The oriented mesh
	 * @param bUseWindingNumber if true, use winding number to determine if a point is inside the surface, otherwise use a ray casting method
	 * @param bInverse if true, the inside and outside will be inversed
	 */
	explicit OrientedSurfaceComponent(const ObjectPtr<StaticMesh>& OrientedMesh, bool bUseWindingNumber = true, bool bInverse = false)
		: bWindingNumber(bUseWindingNumber), Sign(bInverse ? -1. : 1.)
	{
		Build(OrientedMesh, bUseWindingNumber, bInverse);
	}


	/**
	 * Is the point inside the surface
	 * @param Point The point to check
	 * @return True if the point is inside the surface
	 */
	bool Inside(const FVector& Point) const;

	FORCEINLINE bool Outside(const FVector& Point) const
	{
		return !Inside(Point);
	}

	/**
	 * Calculate the distance from the point to the surface
	 * @param Point The point to calculate
	 * @return The distance from the point to the surface
	 */
	double Distance(const FVector& Point) const;

	/**
	 * Calculate the signed distance from the point to the surface
	 * @param Point The point to calculate
	 * @return The signed distance from the point to the surface, if the point is inside the surface, the distance will be negative
	 */
	FORCEINLINE double SignedDistance(const FVector& Point) const
	{
		return Inside(Point) ? -Distance(Point) : Distance(Point);
	}

protected:
	OrientedSurfaceComponent() = default;

	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	void Build(const ObjectPtr<StaticMesh>& OrientedMesh, bool bUseWindingNumber = true, bool bInverse = false);

	bool bWindingNumber = true;

	// Will be -1 when using inverse
	double Sign = 1.;

	// Used for winding number
	igl::FastWindingNumberBVH fwn_bvh;

	igl::AABB<MatrixXd, 3> AABB;
};
