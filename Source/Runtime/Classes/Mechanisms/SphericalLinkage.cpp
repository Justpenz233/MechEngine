//
// Created by MarvelLi on 2024/5/7.
//
#include "SphericalLinkage.h"
#include "Mesh/MeshBoolean.h"

ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateLinkageTo(FVector PortLocation, FVector TargetLocation, bool bIsEffector)
{
	// First remap the target location to the same radius as this
	PortLocation = GetOwner()->GetLocation();
	auto Radius = PortLocation.norm();
	auto InnerRadius = Radius - Thickness * 0.5;
	auto OutterRadius = Radius + Thickness * 0.5;
	if(bIsEffector)
	{
		InnerRadius = Radius - EffectorRadius * 0.5;
		OutterRadius = Radius + EffectorRadius * 0.5;
	}

	double HalfWidth = Width * 0.5f;
	double CenterAngle = acos(1. - Pow2((PortLocation.normalized() - TargetLocation.normalized()).norm()) * 0.5);
	auto   RotationAxis = PortLocation.normalized().cross(TargetLocation.normalized()).normalized();

	static constexpr int SampleRing = 32;
	static constexpr int SampleSlice = 64;

	TArray<FVector>	 Vertex;
	TArray<Vector3i> Face;
	for (int i = 0; i < SampleSlice; i++)
	{
		double t = i / (SampleSlice - 1.);
		FVector CenterPoint = (Eigen::AngleAxisd(CenterAngle * t, RotationAxis) * PortLocation).normalized();
		FVector WidthAxis = RotationAxis.cross(CenterPoint).normalized();
		for (int j = 0; j < SampleRing; j++)
		{
			double t2 = j / (SampleRing - 1.);
			double WidthAngle = Lerp(-HalfWidth, HalfWidth, t2);
			auto   Point =
				AngleAxisd(WidthAngle, WidthAxis) * CenterPoint;
			FVector InnerPoint = Point.normalized() * InnerRadius;
			Vertex.emplace_back(InnerPoint);
		}
		for (int j = 0; j < SampleRing; j++)
		{
			double t2 = j / (SampleRing - 1.);
			double WidthAngle = Lerp(HalfWidth, -HalfWidth, t2);
			auto   Point =
				AngleAxisd(WidthAngle, WidthAxis) * CenterPoint;
			FVector OutterPoint = Point.normalized() * OutterRadius;
			Vertex.emplace_back(OutterPoint);
		}
	}
	int SampleRingCount = SampleRing * 2;
	for (int i = 0; i < SampleSlice - 1; i++)
	{
		for (int j = 0; j < SampleRingCount; j++)
		{
			auto A = i * SampleRingCount + j;
			auto B = i * SampleRingCount + (j + 1) % SampleRingCount;
			auto C = A + SampleRingCount;
			auto D = B + SampleRingCount;
			Face.emplace_back(A, C, B);
			Face.emplace_back(B, C, D);
		}
	}
	ObjectPtr<StaticMesh> Slot;
	if(bIsEffector)
	{
		Slot = BasicShapesLibrary::GenerateSphere(EffectorRadius, 256);
		Slot->Translate(TargetLocation);
	}
	else
	{
		Slot = BasicShapesLibrary::GenerateHollowCylinder(SlotRadius, ConnectorRadius, Thickness, 256);
		FTransform Transform;
		Transform.SetRotation(FQuat::FromTwoVectors(FVector(0, 0, 1), TargetLocation.normalized()));
		Transform.SetTranslation(TargetLocation.normalized() * Radius);
		Slot->TransformMesh(Transform);
	}
	auto Mesh = NewObject<StaticMesh>(Vertex, Face);
	Mesh->FillHoles();
	Mesh = MeshBoolean::MeshUnion(Mesh, Slot);
	Mesh->TransformMesh(GetOwner()->GetFTransform().Inverse());
	return Mesh;
}
ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateSocketMesh()
{
	return BasicShapesLibrary::GenerateCylinder(Thickness * 1.1f, ConnectorRadius, 256);
}

ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateJointMesh()
{
	return BasicShapesLibrary::GenerateHollowCylinder(SlotRadius, ConnectorRadius, Thickness, 256);
}