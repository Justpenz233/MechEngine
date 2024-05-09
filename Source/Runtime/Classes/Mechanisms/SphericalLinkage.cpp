//
// Created by MarvelLi on 2024/5/7.
//
#include "SphericalLinkage.h"

ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateLinkageTo(FVector PortLocation, FVector TargetLocation)
{
	// First remap the target location to the same radius as this
	PortLocation = GetOwner()->GetLocation();
	auto Radius = PortLocation.norm();
	auto InnerRadius = Radius - Thickness * 0.5;
	auto OutterRadius = Radius + Thickness * 0.5;

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
	auto Mesh = NewObject<StaticMesh>(Vertex, Face);
	Mesh->FillHoles();
	Mesh->TransformMesh(GetOwner()->GetFTransform().Inverse().GetMatrix());
	return Mesh;
}
ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateSocketMesh()
{
	return BasicShapesLibrary::GenerateCylinder(Thickness * 1.1f, 0.01f, 256);
}

ObjectPtr<StaticMesh> SphericalLinkageComponent::GenerateJointMesh()
{
	return BasicShapesLibrary::GenerateHollowCylinder(0.03f, 0.01f, Thickness, 256);
}