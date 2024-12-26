//
// Created by MarvelLi on 2024/12/25.
//

#include "SplineLibrary.h"
#include "tinysplinecxx.h"
#include <vector>

namespace SplineLibrary
{
double SplineCurvature(const tinyspline::BSpline& spline, double u, int Dimension)
{
	// Ensure the dimension is valid (2D or 3D)
	if (Dimension != 2 && Dimension != 3) {
		throw std::invalid_argument("Dimension must be 2 or 3.");
	}

	// Clamp u to [0, 1]
	u = std::clamp(u, 0.0, 1.0);

	// Evaluate the first derivative
	auto firstDeriv = spline.derive(1);
	std::vector<tinyspline::real> r1 = firstDeriv.eval(u).result();

	// Evaluate the second derivative
	auto secondDeriv = spline.derive(2);
	std::vector<tinyspline::real> r2 = secondDeriv.eval(u).result();

	// Compute norm of the first derivativew
	double normR1 = 0.0;
	for (int i = 0; i < Dimension; ++i) {
		normR1 += r1[i] * r1[i];
	}
	normR1 = std::sqrt(normR1);

	if (normR1 == 0) {
		throw std::runtime_error("First derivative norm is zero; curvature is undefined.");
	}

	if (Dimension == 2) {
		// Compute curvature for 2D
		double crossProduct = r1[0] * r2[1] - r1[1] * r2[0];
		return std::abs(crossProduct) / (normR1 * normR1 * normR1);
	} else if (Dimension == 3) {
		// Compute curvature for 3D
		std::array<double, 3> crossProduct = {
			r1[1] * r2[2] - r1[2] * r2[1],
			r1[2] * r2[0] - r1[0] * r2[2],
			r1[0] * r2[1] - r1[1] * r2[0]
		};
		double normCross = 0.0;
		for (double val : crossProduct) {
			normCross += val * val;
		}
		normCross = std::sqrt(normCross);

		return normCross / (normR1 * normR1);
	}
}
}