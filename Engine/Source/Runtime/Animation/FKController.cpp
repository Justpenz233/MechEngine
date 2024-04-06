//
// Created by MarvelLi on 2023/12/21.
//

#include "FKController.h"

void FKController::SetSpeed(double NewSpeed)
{
	Speed = NewSpeed;
}

void FKController::Pause()
{
	IsPause = true;
}

void FKController::Run()
{
	IsPause = false;
}

void FKController::Init()
{
	Actor::Init();
	Solver = NewObject<FKSolver>();

	for (auto i : Joints)
		Solver->AddJoint(i->GetJoint());

	Solver->Init();
}

void FKController::BeginPlay()
{
	Actor::BeginPlay();
	Init();
}

void FKController::Tick(double DeltaTime)
{
	if (IsPause)
		return;
	DeltaTime *= Speed;
	Actor::Tick(DeltaTime);

	for (auto i : Joints) {
		if(i->GetJoint()->IsRootJoint()) {
			Solver->Drive(i->GetJoint(), DeltaTime);
			// i->FeatchFromActor();
		}
	}

	bool Success = Solver->Solve();

	if(Success)
	{
		for (auto i : Joints)
			i->ApplyToActor();
	}
	else
		LOG_ERROR("Solve Failed");
}

void FKController::AddJoints(std::vector<ObjectPtr<JointComponent>>&& InJoints)
{
	Joints = InJoints;
}

void FKController::AddJoints(std::vector<ObjectPtr<Actor>>&& InJoints)
{
	for(auto i : InJoints)
	{
		if(auto JointC = i->GetComponent<JointComponent>())
		{
			Joints.push_back(JointC);
		}
	}
}