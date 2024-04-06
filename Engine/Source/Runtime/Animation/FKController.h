//
// Created by MarvelLi on 2023/12/21.
//

#pragma once
#include "FKSolver.h"
#include "JointComponent.h"
#include "Game/Actor.h"

class FKController : public Actor
{
protected:
	ObjectPtr<FKSolver> Solver;

	bool	 IsPause = true;
	double	 Speed = 1.0;
	std::vector<ObjectPtr<JointComponent>> Joints;
public:
	void SetSpeed(double NewSpeed);
	void Pause();
	void Run();

	virtual void Init() override;

	virtual void BeginPlay() override;

	virtual void Tick(double DeltaTime) override;

	void AddJoints(std::vector<ObjectPtr<JointComponent>>&& InJoints);
	void AddJoints(std::vector<ObjectPtr<Actor>>&& InJoints);
};
