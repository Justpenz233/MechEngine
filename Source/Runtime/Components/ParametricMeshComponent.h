//
// Created by MarvelLi on 2024/3/19.
//

#pragma once
#include <igl/exact_geodesic.h>
#include "StaticMeshComponent.h"
#include "Algorithm/GeometryProcess.h"
#include <igl/AABB.h>

/**
 * Abstract class for parametric mesh component
 * Should manage a mesh component and a parametric surface
 */

MCLASS(ParametricMeshComponent)
class ENGINE_API ParametricMeshComponent : public StaticMeshComponent
{
	REFLECTION_BODY(ParametricMeshComponent)
public:
	/**
	* Check if the UV coordinate is valid
	*/
	virtual bool ValidUV(double u, double v) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual void NormlizeUV(double& u, double& v) const
	{
		v = std::clamp(v, 0., 1.);
		u = std::fmod(u, 1.);
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
	/// @math x(u,v) = Directrix(u) + v * DirectorCurve(u),
	/// @see https://mathworld.wolfram.com/RuledSurface.html
	virtual FVector SampleThickness(double u, double v, double ThicknessSample) const
	{
		if(ThicknessSample == 0.) return Sample(u, v);
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

	/***
	 * Project a point to the surface at thickness = 0
	 * \min ||Sample(UV) - Point||
	 * @param Point Position in world space
	 * @return UV coordinate which is closest to the point
	 */
	virtual FVector2 Projection(const FVector& Point) const
	{
		return Algorithm::GeometryProcess::Projection(Point, [&](const FVector2& UV) {
			return Sample(UV.x(), UV.y());
		});
	}
	virtual FVector2 Projection(const FVector& Point, const FVector2& InitialGuess) const
	{
		return Algorithm::GeometryProcess::Projection(Point, [&](const FVector2& UV) {
			return Sample(UV.x(), UV.y());
		}, InitialGuess);
	}

	/***
	 * Project a point to the surface at given thickness
	 * \min ||SampleThickness(U, V, thickness) - Point||
	 * @param Point Position in world space
	 * @param Thickness Thickness to project
	 * @return UV coordinate which is closest to the point
	 */
	virtual FVector2 ProjectionThickness(const FVector& Point, double Thickness) const
	{
		if(Thickness == 0.) return Projection(Point);
		ASSERTMSG(false, "Not implemented");
		return {};
	}
	virtual FVector2 ProjectionThickness(const FVector& Point, const FVector2& InitialGuess, double Thickness) const
	{
		if(Thickness == 0.) return Projection(Point, InitialGuess);
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	/***
	 * Project a point to the surface at outter surface
	 * i.e. Thickness = MeshThickness * 0.5
	 */
	virtual FVector2 ProjectionThickness(const FVector& Point) const
	{
		if (MeshThickness == 0.) return Projection(Point);
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
		MeshThickness = InThickness;
		MarkAsDirty(DIRTY_REMESH);
	}

	virtual double GetThickness() const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const
	{
		ASSERTMSG(!Start.hasNaN() && !End.hasNaN(), "Start or End has NaN");
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

	virtual void PostEdit(Reflection::FieldAccessor& Field) override
	{
		StaticMeshComponent::PostEdit(Field);
		if (Field == NAME(MeshThickness))
			SetThickness(MeshThickness);
	}

	virtual ObjectPtr<StaticMesh> GetZeroThicknessMesh() const
	{
		ASSERT("Not implemented");
		return nullptr;
	};

protected:
	ParametricMeshComponent() = default;

	MPROPERTY()
	double MeshThickness = 0.;
};
