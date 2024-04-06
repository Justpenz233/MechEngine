//
// Created by MarvelLi on 2023/12/21.
//

#pragma once
#include "FKController.h"
#include "IKSolver.h"

template <typename T = IKSolver>
class IKController : public FKController
{
public:
	virtual void Init() override;
};

template <typename T>
void IKController<T>::Init()
{
	Actor::Init();
	Solver = NewObject<T>();

	for (auto i : Joints)
		Solver->AddJoint(i->GetJoint());

	Solver->Init();
}