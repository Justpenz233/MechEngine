//
// Created by MarvelLi on 2024/12/25.
//

#pragma once
namespace tinyspline
{
	class BSpline;
};
namespace SplineLibrary
{
/**
 * Calculate the curvature of a spline at a given parameter in terms of parameter u.(Not length)
 * @param spline The spline to calculate the curvature of.
 * @param u The parameter at which to calculate the curvature.
 * @param Dimension The dimension of the spline (2D or 3D).
 * @return The curvature of the spline at the given parameter.
 */
double SplineCurvature(const tinyspline::BSpline& spline, double u, int Dimension = 3);

};