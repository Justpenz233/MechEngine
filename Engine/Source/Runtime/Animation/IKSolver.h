#pragma once
#include "Animation/FKSolver.h"
#include "Core/CoreMinimal.h"
#include "Animation/Joints.h"
#include "Game/Actor.h"

#include <unsupported/Eigen/src/NumericalDiff/NumericalDiff.h>

template <typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct IKBaseFunctor
{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	IKBaseFunctor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	IKBaseFunctor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

};

// Solver for full body IK and FK
// First Init() will set the relative topology and transform, CalcLocal()
// Use Levenberg-Marquardt-Optimization of Eigen lib
struct IKLossFunction : IKBaseFunctor<double>
{
	IKLossFunction(int ParameterDimension)
	: IKBaseFunctor<double>(ParameterDimension, 12), ParameterDimension(ParameterDimension) {}
	int ParameterDimension;
	std::vector<ObjectPtr<Joint>> Joints;

	/// \brief calc loss of full body ik
	/// \param x Parameter of full body
	/// \param fvec Loss vector for each dimention, shoule be 4 * 3 of a transform matrix
	/// \return 0 if successful
	int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const;
	int df(const VectorXd &x, MatrixXd &fjac) const;


	mutable double LossHistory;
};

/// \brief An IKSolver based on Levenberg-Marquardt-Optimization of Eigen lib
/// Cannot perform animation, only solve IK
/// @see IKController
class IKSolver : public FKSolver
{
protected:
	VectorXd PreParameter;

public:
	double TranslationWeight = 1.;
	double eps = 1e-4;
	virtual void Init() override;

	/// \brief Solve IK, and return if the solve is successful and one solution as parameter
    virtual double Solve() override;

	/// \brief Gather current parameter from system
	/// \return Parmeter vector
	VectorXd GatherParameter() const;
	int ParameterNum() const;
};
