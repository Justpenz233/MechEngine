//
// Created by MarvelLi on 2024/7/11.
//
#pragma once

#include "ParametricMeshComponent.h"
#include "igl/AABB.h"
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
		bool	Valid;
		int		TriangleIndex;
		double	u, v;
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
		SetMeshData(Algorithm::GeometryProcess::SolidifyMeshEven(DisplayMesh, MeshThickness));
	}

	virtual FVector2 Projection(const FVector& Point) const override;

	virtual ObjectPtr<StaticMesh> GetUVMesh() const
	{
		ASSERT("Not implemented");
		return nullptr;
	}

	virtual ObjectPtr<StaticMesh> GetZeroThicknessMesh() const override
	{
		return DisplayMesh;
	}

protected:
	ObjectPtr<StaticMesh> PMesh;
	ObjectPtr<StaticMesh> DisplayMesh;

	igl::AABB<MatrixX3d, 3> AABB;
							ParametricAlgorithmComponent() = default;
};

inline FVector2 ParametricAlgorithmComponent::Projection(const FVector& Point) const
{
	ASSERTMSG(PMesh->HasValidUV(), "AABB Mesh has no valid UV");
	RowVector3d ClosetPoint; int TriangleIndex;
	AABB.squared_distance(PMesh->GetVertices(), PMesh->GetTriangles(), Point, TriangleIndex, ClosetPoint);
	auto Tri = PMesh->GetTriangle(TriangleIndex);
	MatrixX3d Bary;
	igl::barycentric_coordinates(ClosetPoint,
		PMesh->GetVertex(Tri[0]).transpose(),
		PMesh->GetVertex(Tri[1]).transpose(),
		PMesh->GetVertex(Tri[2]).transpose(), Bary);
	auto UV0 = PMesh->GetUV(Tri[0]); auto UV1 = PMesh->GetUV(Tri[1]); auto UV2 = PMesh->GetUV(Tri[2]);
	return UV0 * Bary(0,0) + UV1 * Bary(0, 1) + UV2 * Bary(0, 2);
}
