//
// Created by MarvelLi on 2024/1/16.
//
#include "ParametricSurface.h"
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include "CoreMinimal.h"
#include "Math/Random.h"

struct SurfaceProjectFunctor
{
    using Scalar = double;
    enum {
        InputsAtCompileTime = 2, // UV
        ValuesAtCompileTime = 3  // distance to [x, y, z]
    };
    typedef Eigen::Matrix<double,Eigen::Dynamic,1> InputType;
    typedef Eigen::Matrix<double,Eigen::Dynamic,1> ValueType;
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> JacobianType;
    typedef std::function<FVector(double u, double v)> SurfaceSampleFunc;

    SurfaceSampleFunc SampleFunc;
    FVector Target;
    const int m_inputs, m_values;

    SurfaceProjectFunctor(SurfaceSampleFunc&& InSurface, const FVector& InTarget) : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime),
         Target(InTarget), SampleFunc(InSurface) {}

    int inputs() const { return m_inputs; }
    int values() const { return m_values; }

    int operator()(const Eigen::VectorXd &UV, Eigen::VectorXd &DisXYZ) const
    {
        double U = UV(0);
        double V = UV(1);

        double Penalty = 0.;
        if(V < 0. || V > 1.) Penalty += 1000.;
        if(U < 0. || U > 1.) Penalty += 1000.;

        V = std::clamp(V, 0., 1.);
        U = std::clamp(U, 0., 1.);
        Vector3d Pos = SampleFunc(U, V);

        DisXYZ.resize(3);
        DisXYZ(0) = std::pow(Pos.x() - Target.x(), 2) + Penalty;
        DisXYZ(1) = std::pow(Pos.y() - Target.y(), 2) + Penalty;
        DisXYZ(2) = std::pow(Pos.z() - Target.z(), 2) + Penalty;
        return 0;
    }
};


Vector2d ParametricSurface::Projection(const FVector& Pos) const
{
    SurfaceProjectFunctor fucnctor([this](double u, double v) {
        return Sample(u, v);
    }, Pos);
    double BestEnergy = 1e6;
    Vector2d Best;
	for(double u = 0.; u <= 1.; u += 0.1)
	{
		for (double v = 0.; v <= 1.; v += 0.1)
		{
			VectorXd UV(2);
			UV << u, v;
			Eigen::NumericalDiff<SurfaceProjectFunctor> numDiff(fucnctor);
			Eigen::LevenbergMarquardt<Eigen::NumericalDiff<SurfaceProjectFunctor>, double> lm(numDiff);
			int t = lm.minimize(UV);

			double Energy = (Sample(u, v) - Pos).norm();
			if(Energy < BestEnergy)
			{
				BestEnergy = Energy;
				Best = {u, v};
			}
			Energy = (Sample(UV(0), UV(1)) - Pos).norm();
			if(Energy < BestEnergy)
			{
				BestEnergy = Energy;
				Best = {UV(0), UV(1)};
			}
		}
	}
    return Best;
}
