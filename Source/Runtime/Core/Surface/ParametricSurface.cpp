//
// Created by MarvelLi on 2024/1/16.
//
#include "ParametricSurface.h"
#include "CoreMinimal.h"
#include "Algorithm/GeometryProcess.h"

Vector2d ParametricSurface::Projection(const FVector& Pos) const
{
	return Algorithm::GeometryProcess::Projection(Pos, [&](const FVector2& UV) {
		return Sample(UV[0], UV[1]);
	});
}

Vector2d ParametricSurface::ProjectionThickness(const FVector& Pos, double Thickness) const
{
	return Algorithm::GeometryProcess::Projection(Pos, [&](const FVector2& UV) {
		return SampleThickness(UV[0], UV[1], Thickness);
	});
}
