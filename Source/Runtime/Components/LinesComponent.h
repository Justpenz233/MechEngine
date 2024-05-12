//
// Created by MarvelLi on 2024/5/12.
//

#pragma once
#include "RenderingComponent.h"

class LinesComponent : public RenderingComponent
{
public:
	/**
	 * Draw a point with the given radius and color in world. Will last for the given lifetime.
	 * The point will be persistent if lifetime is negative.
	 * @param WorldPosition world position of the point
	 * @param Radius radius of the point, in pixel
	 * @param Color color of the point
	 * @param LifeTime lifetime of the point
	 */
	void AddPoint(const FVector& WorldPosition, double Radius, const FColor& Color, double LifeTime = -1.);
};
