//
// Created by MarvelLi on 2024/7/11.
//
#pragma once

#include "ParametricMeshComponent.h"
#include "Core/CoreMinimal.h"

MCLASS(ParametricAlgorithmComponent)
class ParametricAlgorithmComponent : public ParametricMeshComponent
{
	REFLECTION_BODY(ParametricAlgorithmComponent)
public:
	ParametricAlgorithmComponent(const ObjectPtr<StaticMesh>& InDisplayMesh, const ObjectPtr<StaticMesh>& InPMesh)
	{
		DisplayMesh = InDisplayMesh;
		PMesh = InPMesh;
		SetMeshData(DisplayMesh);
	}

	struct UVMappingSampleResult
	{
		bool Valid;
		int TriangleIndex;
		double u, v;
		FVector Position;
	};

	virtual UVMappingSampleResult SampleHit(double U, double V) const
	{
		ASSERTMSG(false, "Not implemented");
		return {};
	}

	virtual FVector Sample(double U, double V) const override
	{
		NormlizeUV(U, V);
		return SampleHit(U, V).Position;
	}

	virtual FVector SampleNormal(double u, double v) const override
	{
		auto Hit = SampleHit(u, v);

		// calc vertex normal by barycentric interpolation
		auto T1 = PMesh->triM(Hit.TriangleIndex, 0);
		auto T2 = PMesh->triM(Hit.TriangleIndex, 1);
		auto T3 = PMesh->triM(Hit.TriangleIndex, 2);
		auto N1 = PMesh->VertexNormal.row(T1);
		auto N2 = PMesh->VertexNormal.row(T2);
		auto N3 = PMesh->VertexNormal.row(T3);
		return N1 * (1. - Hit.u - Hit.v) + N2 * Hit.u + N3 * Hit.v;
	}

	virtual bool ValidUV(double U, double V) const override
	{
		NormlizeUV(U, V);
		auto Hit = SampleHit(U, V);
		return Hit.Valid;
	}

	virtual void Remesh() override
	{
		ParametricMeshComponent::Remesh();
		SetMeshData(Algorithm::GeometryProcess::SolidifyMesh(DisplayMesh, MeshThickness));
	}

protected:
	ObjectPtr<StaticMesh> PMesh;
	ObjectPtr<StaticMesh> DisplayMesh;

	ParametricAlgorithmComponent() = default;
};
