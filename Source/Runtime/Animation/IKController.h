//
// Created by MarvelLi on 2023/12/21.
//

#pragma once
#include "FKController.h"
#include "IKSolver.h"
#include "Math/LinearAlgebra.h"

class IKController : public FKController
{
public:

	template <class T> requires std::is_base_of_v<FKSolver, T>
	explicit IKController(const ObjectPtr<T>& InSolver)
		: FKController(InSolver) {}

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