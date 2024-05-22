//
// Created by MarvelLi on 2024/3/17.
//

#pragma once
#include "Game/Actor.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/tri.h>
#include "Components/ParametricMeshComponent.h"


struct UVMappingMeshResult
{
	bool Valid;
	int TriangleIndex;
	double u, v;
	FVector Position;
};

/**
 * This component support a bijective mapping from 3D position to UV coordinate
 * Require the original mesh is an open mesh
 * Support sample a UV cordiante and return the 3D position (UV coordinate maybe invalid)
 */
MCLASS(SCAFParametricMeshComponent)
class ENGINE_API SCAFParametricMeshComponent : public ParametricMeshComponent
{
	using BVHNode = bvh::v2::Node<double, 3>;
	REFLECTION_BODY(SCAFParametricMeshComponent)
private:
	SCAFParametricMeshComponent() = default;

public:
	// UV Mesh, store the UV information of vertices
	MatrixX3d UVMesh;
	MatrixX3d Vertices;
	MatrixX3i Indices;

	MPROPERTY()
	bool bShowUV = false; // Show uv

	SCAFParametricMeshComponent(ObjectPtr<StaticMesh> InitMesh);

	// Calc UV correspond to 3D position in model space
	UVMappingMeshResult SampleHit(double U, double V) const;

	FORCEINLINE virtual FVector Sample(double U, double V) const override;

	// Use mesh vertex normal
	virtual FVector SampleNormal(double u, double v) const override;

	FORCEINLINE virtual bool ValidUV(double U, double V) const override;

	FORCEINLINE ObjectPtr<StaticMesh> GetUVMesh();

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const override;

private:
	// BVH tree which store the UV mesh, used to fast sample UV
	bvh::v2::Bvh<BVHNode> BVHUVMesh;

	// Triangle structure
	TArray<bvh::v2::PrecomputedTri<double>> Triangles;
};


FORCEINLINE FVector SCAFParametricMeshComponent::Sample(double U, double V) const
{
	// The SCAFP UV space is a torus, so U could be negative or greater than 1, normalize first
	if(U > 1.) U -= 1.; if(U < 0.) U += 1.;
	auto Hit = SampleHit(U, V);
	if(Hit.Valid)
		return SampleHit(U, V).Position;
	else
		return FVector::Zero();
}

FORCEINLINE bool SCAFParametricMeshComponent::ValidUV(double U, double V) const
{
	return SampleHit(U, V).Valid;
}

FORCEINLINE ObjectPtr<StaticMesh> SCAFParametricMeshComponent::GetUVMesh()
{
	auto Mesh = NewObject<StaticMesh>(UVMesh, Indices);
	return Mesh;
}