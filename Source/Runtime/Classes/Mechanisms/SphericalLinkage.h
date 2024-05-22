//
// Created by MarvelLi on 2024/5/7.
//

#pragma once
#include "SpatialJoints.h"

class ENGINE_API SphericalLinkageComponent : public SpatialJointComponent
{
public:
	static constexpr double Thickness = 0.03;
	static constexpr double Width = 0.08; // in radian
	static constexpr double SlotRadius = 0.04;
	static constexpr double ConnectorRadius = 0.01;
	static constexpr double EffectorRadius = 0.01;
	static constexpr double EffectorWidth = 0.02;

	
	SphericalLinkageComponent(const FTransform& InInitTransform, bool InIsRoot)
		: SpatialJointComponent(FreeZ, FreeNone, InInitTransform, InIsRoot) {}

	virtual ObjectPtr<StaticMesh> GenerateLinkageTo(FVector PortLocation, FVector TargetLocation, bool bIsEffector) override;
	virtual ObjectPtr<StaticMesh> GenerateSocketMesh() override;
	virtual ObjectPtr<StaticMesh> GenerateJointMesh() override;

};

class ENGINE_API SphericalLinkageActor : public Actor
{
public:
	SphericalLinkageActor(const FTransform& InInitTransform, bool InIsRoot = false) : Actor(InInitTransform)
	{
		Component = AddComponent<SphericalLinkageComponent>(InInitTransform, InIsRoot);
	}

	auto GetJointComponent() { return Component; }

	template <typename T> requires std::is_base_of_v<Joint, T>
	void AddNextJoint(ObjectPtr<T> NextJoint)
	{
		Component->GetJoint()->AddNextJoint(NextJoint);
	}

	template <typename T> requires std::is_base_of_v<SpatialJointComponent, T>
	void AddNextJoint(ObjectPtr<T> NextJoint)
	{
		Component->AddNextJoint(NextJoint);
		Component->GetJoint()->AddNextJoint(NextJoint->GetJoint());
	}

	template <typename T> requires requires(T a) { a.GetJointComponent(); }
	void AddNextJoint(const ObjectPtr<T>& NextJoint)
	{
		Component->GetJoint()->AddNextJoint(NextJoint->GetJointComponent()->GetJoint());
		Component->AddNextJoint(NextJoint->GetJointComponent());
	}

protected:
	ObjectPtr<SphericalLinkageComponent> Component;
};