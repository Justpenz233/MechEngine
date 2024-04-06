#include "FKSolver.h"
#include <queue>

FKSolver::FKSolver()
{ }

FKSolver::FKSolver(TArray<ObjectPtr<Joint>>&& InJoints)
{
	AddJoints(InJoints);
}

FKSolver::~FKSolver()
{}

void FKSolver::Init()
{
    SortJoints();
    for(auto Joint: Joints)
        Joint->CalcLocal();
}

double FKSolver::Solve()
{
    for(auto Joint: Joints)
    	if(!Joint->IsRootJoint())
    		Joint->CalcGlobal();
	return true;
}

void FKSolver::SortJoints()
{
	std::queue<ObjectPtr<Joint>> Q;
	std::set<ObjectPtr<Joint>>	 Visit;
	// Find Roots
	for (auto i : JointsSet)
	{
		if (i->IsRootJoint())
		{
			Q.push(i);
			Visit.insert(i);
			Roots.push_back(i);
		}
	}

	while (!Q.empty())
	{
		ObjectPtr<Joint> Head = Q.front();
		for (auto Next : Head->NextJoints)
		{
			auto Temp = Next.lock();
			if (!Visit.count(Temp))
			{
				Q.push(Temp);
				Visit.insert(Temp);
			}
		}
		Joints.push_back(Head);
		Q.pop();
	}
	Visit.clear();
}

void FKSolver::AddJoints(TArray<ObjectPtr<Joint>> InJoint)
{
	for (auto i : InJoint)
		JointsSet.insert(i);
}