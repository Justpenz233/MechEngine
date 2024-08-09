//
// Created by MarvelLi on 2023/12/21.
//

#pragma once
#include "FKController.h"
#include "IKSolver.h"
#include "Math/LinearAlgebra.h"

template <typename T = IKSolver>
class IKController : public FKController
{
public:
	virtual void Init() override;

	struct MotionSequence
	{
		TMap<Joint*, TArray<FTransform>> JointTransforms; // <Joint ptr, {Transforms}>
		MotionSequence(MotionSequence&&) = default;
		MotionSequence(const MotionSequence&) = default;
		MotionSequence& operator=(const MotionSequence&) = default;
		MotionSequence() = default;
	};
	/**
	 * Simulate a loop and return the sequence of all the actors controlled by this controller
	 * @param TotalFrames The frames for simulation
	 * @param TotalTime The total time for simulation
	 * @param DrivenFunction The function to drive the joints
	 * @return A sequence of all the actors
	 */
	MotionSequence GetActorSequence(int TotalFrames, double TotalTime, const TFunction<void(Joint*, double)>& DrivenFunction);


	struct SimulatedMotion
	{
		TArray<FVector> Trajectory;
		TMap<Joint*, TArray<FTransform>> JointTransforms; // <Joint ptr, {Transforms}>
		SimulatedMotion(SimulatedMotion&&) = default;
		SimulatedMotion(const SimulatedMotion&) = default;
		SimulatedMotion& operator=(const SimulatedMotion&) = default;
		SimulatedMotion() = default;
	};
	/**
	 * Simulate a loop and return the trajectory of the effector
	 * @param Effector The effector which to extract the trajectory
	 * @param Frames The frames for simulation
	 * @param DrivenFunction The function to drive the joints
	 * @return The trajectory of the effector
	 */
	SimulatedMotion GetSimulatedTrajectory(JointComponent* Effector, int Frames, const TFunction<void(Joint*)>& DrivenFunction);

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

template <typename T>
typename IKController<T>::MotionSequence IKController<T>::GetActorSequence(int TotalFrames, double TotalTime, const TFunction<void(Joint*, double)>& DrivenFunction)
{
	Init();
	MotionSequence Result;

	TArray<JointComponent*> Roots;
	TArray<FTransform> RootsBeforeTransfom;

	TArray<FVector> Trajectory;
	double TimePerFrame = TotalTime / (TotalFrames - 1);

	for (auto i : Joints) {
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

template <typename T>
typename IKController<T>::SimulatedMotion IKController<T>::GetSimulatedTrajectory(JointComponent* Effector, int Frames, const TFunction<void(Joint*)>& DrivenFunction)
{
	if(Effector == nullptr)
		return {};

	Init();
	SimulatedMotion Result;

	TArray<JointComponent*> Roots;
	TArray<FTransform> RootsBeforeTransfom;

	TArray<FVector> Trajectory;
	for (auto i : Joints) {
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
