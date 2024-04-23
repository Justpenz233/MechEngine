//
// Created by MarvelLi on 2024/3/19.
//

#pragma once
#include <igl/exact_geodesic.h>

#include "StaticMeshComponent.h"

/**
* Abstract class for parametric mesh component
* Should manage a mesh component and a parametric surface
*/

MCLASS(ParametricMeshComponent)
class ParametricMeshComponent : public StaticMeshComponent
{
	REFLECTION_BODY(ParametricMeshComponent)
protected:
	ParametricMeshComponent() = default;

public:

	/**
	* Check if the UV coordinate is valid
	*/
	virtual bool ValidUV(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	//Sample at inner surface (thickness = 0)
	virtual FVector Sample(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	//Sample at outter surface
	virtual FVector SampleThickness(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	/// Override this method Is ALL YOU NEED
	/// @math x(u,v) = Directrix(u) + v * DirectorCur    ve(u),
	/// @see https://mathworld.wolfram.com/RuledSurface.html
	virtual FVector SampleThickness(double u, double v, double ThicknessSample) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	/// Return normalized vertex normal
	/// @math (Sample(u + 0.01, v) - Sample(u - 0.01, v)) X (Sample(u, v + 0.01) - Sample(u, v - 0.01)) norm
	virtual FVector SampleNormal(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual FVector SampleTangent(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual FVector SampleTangentU(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual FVector SampleTangentV(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	/**
	 * Project a point to the surface
	 * \min ||Sample(UV) - Point||
	 * @param Point Position in world space
	 * @return UV coordinate which is closest to the point
	 */
	virtual FVector2 Projection(const FVector& Point) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual double SDF(const FVector& Point) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual void SetThickness(double InThickness)
	{
		ASSERTMSG(false, "Not implemented");
	}

	virtual double GetThickness() const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const
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
};