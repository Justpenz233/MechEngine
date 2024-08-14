//
// Created by MarvelLi on 2024/8/14.
//
#include "IKController.h"

IKController::MotionSequence IKController::GetActorSequence(int TotalFrames, double TotalTime, const TFunction<void(Joint*, double)>& DrivenFunction)
{
	Init();
	MotionSequence Result;

	TArray<JointComponent*> Roots;
	TArray<FTransform> RootsBeforeTransfom;

	TArray<FVector> Trajectory;
	double TimePerFrame = TotalTime / (TotalFrames - 1);

	for (const auto& i : Joints) {
		if (i->GetJoint()->IsRootJoint()) {
			Roots.push_back(i.get());
			RootsBeforeTransfom.push_back(i->GetJoint()->GlobalTransform);
		}
		Result.JointTransforms[i->GetJoint().get()] = {};
	}

	for (int i = 0; i < TotalFrames; i++) {
		for (auto& Joint : Joints) {
			Result.JointTransforms[Joint->GetJoint().get()].push_back(Joint->GetJoint()->GlobalTransform);
		}
		for (auto Root : Roots) {
			if(DrivenFunction)
				DrivenFunction(Root->GetJoint().get(), TimePerFrame);
			else
				Solver->Drive(Root->GetJoint(), TimePerFrame);
		}
		Solver->Solve();
	}

	// Restore the root joint
	for (int i = 0; i < Roots.size(); i++)
		Roots[i]->GetJoint()->GlobalTransform = RootsBeforeTransfom[i];
	Solver->Solve();

	return std::move(Result);
}

IKController::SimulatedMotion IKController::GetSimulatedTrajectory(JointComponent* Effector, int Frames, const TFunction<void(Joint*)>& DrivenFunction)
{
	if(Effector == nullptr)
		return {};

	Init();
	SimulatedMotion Result;

	TArray<JointComponent*> Roots;
	TArray<FTransform> RootsBeforeTransfom;

	TArray<FVector> Trajectory;
	for (const auto& i : Joints) {
		if (i->GetJoint()->IsRootJoint()) {
			Roots.push_back(i.get());
			RootsBeforeTransfom.push_back(i->GetJoint()->GlobalTransform);
		}
		Result.JointTransforms[i->GetJoint().get()] = {};
	}
	// Calculate the trajectory
	for (int i = 0; i < Frames; i++) {
		for (auto Root : Roots) {
			DrivenFunction(Root->GetJoint().get());
		}
		Solver->Solve();
		Result.Trajectory.push_back(Effector->GetJoint()->GlobalTransform.GetLocation());
		for (auto& joint : Joints) {
			Result.JointTransforms[joint->GetJoint().get()].push_back(joint->GetJoint()->GlobalTransform);
		}
	}

	// Restore the root joint
	for (int i = 0; i < Roots.size(); i++)
		Roots[i]->GetJoint()->GlobalTransform = RootsBeforeTransfom[i];
	Solver->Solve();

	return std::move(Result);
}
