//
// Created by MarvelLi on 2023/12/28.
//

#include "ClosedChainIKSolver.h"
#include "Animation/IKJoint.h"
#include <unsupported/Eigen/NonLinearOptimization>
#include "Mechanisms/SpatialJoints.h"
#include "Math/LinearAlgebra.h"
#include "Misc/Config.h"

int ClosedChainMechFunctor::operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const
{
	Matrix4d Result = Matrix4d::Zero();
	VectorXd Parameter = x.eval();

	// Set parameter for all IK joints
	for (auto i : Joints)
	{
		if(!i->IsRootJoint())
			if (auto IKjoint = Cast<IKJoint>(i))
				IKjoint->SetParameter(Parameter);
	}

	// Should consume all parameter dimension
	ASSERTMSG(Parameter.size() == 0, "Parameter size not match");

	ObjectPtr<Joint> Driven;
	// FK
	for (auto i : Joints)
	{
		if (!i->IsRootJoint()) // Driven joint use Global from fetch
			i->CalcGlobal();
		else
			Driven = i;
	}
	ObjectPtr<Joint> Ground = Driven->GetParentJoint();

	Result = Ground->GlobalTransform.GetMatrix() - Ground->InitTransform.GetMatrix();
	fvec.resize(values());
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			fvec(i * 4 + j) = Result(i, j);
		}
	}
	return 0;
}

int ClosedChainMechFunctor::df(const VectorXd& x, MatrixXd& fjac) const
{
	fjac.resize(values(), inputs());
	fjac.setZero();

	VectorXd epc;
	double	 tol = 0.0001;
	epc.resize(inputs());
	Eigen::VectorXd LossPlus, LossMinus;
	for (int itr = 0; itr < inputs(); itr++)
	{
		epc.setZero();
		epc(itr) = tol;
		operator()(x + epc, LossPlus);
		operator()(x - epc, LossMinus);
		fjac.col(itr) = (LossPlus - LossMinus) / (2.0 * tol);
	}
	return 0;
}

double ClosedChainIKSolver::Solve()
{
	VectorXd Parameter = PreParameter; // Use parameter from previor solve as init
	ASSERTMSG(Parameter.size() == ParameterNum(), "Parameter size not match");
	ASSERTMSG(Parameter.size() != 0, "Parameter can not be Empty");
	ClosedChainMechFunctor LossFunc(ParameterNum());
	LossFunc.Joints = Joints;
	Eigen::LevenbergMarquardt<ClosedChainMechFunctor, double> lm(LossFunc);
	const auto MechanismSolveTolerance = SimulationEps;
	lm.parameters.ftol = MechanismSolveTolerance;
	lm.parameters.epsfcn = MechanismSolveTolerance;
	int ret = lm.minimize(Parameter);
	ASSERTMSG(Parameter.size() == ParameterNum(), "Parameter size not match");

	Eigen::VectorXd LossVector;
	LossFunc(Parameter, LossVector);
	Loss = std::max(LossVector.maxCoeff(), -LossVector.minCoeff());
	const bool Valid = (Loss < SimulationEps);
	Singularity = CalcSingularity(Parameter);

	PreParameter = Parameter.eval();

	// Apply result parameter to system
	for (auto i : Joints)
		if (auto IKjoint = Cast<IKJoint>(i))
			IKjoint->SetParameter(Parameter);

	ASSERTMSG(Parameter.size() == 0, "Parameter size not match");
	for (auto Joint : Joints)
	{
		if (!Joint->IsRootJoint())
			Joint->CalcGlobal();
	}
	return Loss;
}

void ClosedChainIKSolver::Drive(ObjectPtr<Joint> Root, double DeltaTime)
{
	Root->GlobalTransform.AddRotationLocal(MMath::QuaternionFromEulerXYZ(
{0, 0, DeltaTime}));
}

double ClosedChainIKSolver::CalcSingularity(const VectorXd& Parameter) const
{
	MatrixXd jacFK;
	ClosedChainMechFunctor LossFunc(ParameterNum());
	LossFunc.Joints = Joints;
	LossFunc.df(Parameter, jacFK);
	Eigen::JacobiSVD<MatrixXd> svd(jacFK, Eigen::ComputeThinU | Eigen::ComputeThinV);
	return svd.singularValues().minCoeff();
}
