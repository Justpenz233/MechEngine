//
// Created by marvel on 9/8/24.
//

#pragma once
#include "Components/ActorComponent.h"
#include "Mesh/StaticMesh.h"
#include "igl/fast_winding_number.h"

MCLASS(OrientedSurfaceComponent)
class OrientedSurfaceComponent : public ActorComponent
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

protected:
	OrientedSurfaceComponent() = default;

	void Build(const ObjectPtr<StaticMesh>& OrientedMesh, bool bUseWindingNumber = true, bool bInverse = false);

	bool bWindingNumber;

	// Will be -1 when using inverse
	double Sign = 1.;

	// Used for winding number
	igl::FastWindingNumberBVH fwn_bvh;
};
