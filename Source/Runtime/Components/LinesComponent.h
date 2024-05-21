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



	/**
	 * Draw a line with the given thickness and color in world. Will last for the given lifetime.
	 * The line will be persistent if lifetime is negative.
	 * @param WorldStart world position of the start point of the line
	 * @param WorldEnd world position of the end point of the line
	 * @param Color color of the line
	 * @param Thickness thickness of the line, in pixel
	 * @param LifeTime lifetime of the line
	 */
	void AddLine(const FVector& WorldStart, const FVector& WorldEnd, const FColor& Color, double Thickness = 2., double LifeTime = -1.);
};
