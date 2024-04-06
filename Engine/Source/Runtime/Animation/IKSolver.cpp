#include "IKSolver.h"
#include "IKJoint.h"
#include <unsupported/Eigen/NonLinearOptimization>

int IKLossFunction::operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const
{
	Matrix4d Result = Matrix4d::Zero();
	LossHistory = 0.;
	VectorXd Parameter = x.eval();

	// Set parameter for all IK joints
	for (auto i : Joints)
		if (auto IKjoint = Cast<IKJoint>(i))
			IKjoint->SetParameter(Parameter);

	// Should consume all parameter dimension
	ASSERTMSG(Parameter.size() == 0, "Parameter size not match");

	// FK
	for (auto i : Joints)
		i->CalcGlobal();

	// Calc loss
	for (auto i : Joints)
		if (auto IKjoint = Cast<IKJoint>(i))
			Result += IKjoint->CalcLoss();

	fvec.resize(values());
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			fvec(i * 4 + j) = Result(i, j);
			LossHistory += std::pow(fvec(i * 4 + j), 2);
		}
	}
	return 0;
}

int IKLossFunction::df(const VectorXd& x, MatrixXd& fjac) const
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

void IKSolver::Init()
{
	FKSolver::Init();
	PreParameter = GatherParameter();
}

double IKSolver::Solve()
{
	VectorXd Parameter = PreParameter; // Use parameter from previor solve as init
	ASSERTMSG(Parameter.size() == ParameterNum(), "Parameter size not match");

	IKLossFunction LossFunc(ParameterNum());
	LossFunc.Joints = Joints;

	Eigen::LevenbergMarquardt<IKLossFunction, double> lm(LossFunc);
	int ret = lm.minimize(Parameter);
	ASSERTMSG(Parameter.size() == ParameterNum(), "Parameter size not match");

	const bool Valid = ((ret == 2) && (LossFunc.LossHistory < eps));
	if(!Valid) // Recover parameter
		Parameter = PreParameter;
	PreParameter = Parameter.eval();

	LOG_INFO("Return type: {}  Loss: {}",ret, LossFunc.LossHistory);

	//Apply result parameter to system
	for(auto i : Joints)
		if(auto IKjoint = Cast<IKJoint>(i))
			IKjoint->SetParameter(Parameter);

	ASSERTMSG(Parameter.size() == 0, "Parameter size not match");
	for(auto Joint: Joints)
		if(!Joint->IsRootJoint())
			Joint->CalcGlobal();

	return Valid;
}

VectorXd IKSolver::GatherParameter() const
{
	VectorXd Result(ParameterNum());
	int ParameterCount = 0;
	for(auto i : Joints)
	{
		if(auto IK = Cast<IKJoint>(i))
		{
			if(IK->ParameterNum() == 0) continue;
			VectorXd Parameter = IK->GetParameter();
			Result.segment(ParameterCount, Parameter.size()) = Parameter;
			ParameterCount += Parameter.size();
		}
	}
	return Result;
}

int IKSolver::ParameterNum() const
{
	int Num = 0;
	for(auto i : Joints)
		if (const auto IK = Cast<IKJoint>(i))
			Num += IK->ParameterNum();
	return Num;
}