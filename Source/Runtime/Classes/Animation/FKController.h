//
// Created by MarvelLi on 2023/12/21.
//

#pragma once
#include "FKSolver.h"
#include "JointComponent.h"
#include "Game/Actor.h"

class ENGINE_API FKController : public Actor
{
protected:
	ObjectPtr<FKSolver> Solver;

	bool	 IsPause = true;
	double	 Speed = 1.0;
	std::vector<ObjectPtr<JointComponent>> Joints;

public:
	FKController() = default;

	template <class T> requires std::is_base_of_v<FKSolver, T>
	explicit FKController(const ObjectPtr<T>& InSolver)
		: Solver(Cast<FKSolver>(InSolver)) {}

	void SetSpeed(double NewSpeed);
	void Pause();
	void Run();

	virtual void Init() override;

	virtual void BeginPlay() override;

	virtual void Tick(double DeltaTime) override;

	void AddJoints(std::vector<ObjectPtr<JointComponent>>&& InJoints);
	void AddJoints(std::vector<ObjectPtr<Actor>>&& InJoints);

	template <class T> requires std::is_base_of_v<FKSolver, T>
	void SetSolver(const ObjectPtr<T>& InSolver)
	{
		Solver = Cast<FKSolver>(InSolver);
	}


	template <class T> requires std::is_base_of_v<Actor, T>
	void AddJoints(const std::vector<ObjectPtr<T>>& InJoints)
	{
		for(auto i : InJoints)
		{
			if(auto JointC = i->template GetComponent<JointComponent>())
			{
				Joints.push_back(JointC);
			}
		}
	}
};
