//
// Created by MarvelLi on 2024/6/6.
//

#pragma once
#include "ParametricMeshComponent.h"
#include "Core/CoreMinimal.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/tri.h>
/***
 * Spereical Conformal Parametric Mesh Component
 * This component is used to generate a parametric mesh by conformal mapping to a sphere
 * The input mesh should be a closed mesh and genus 0
 */

MCLASS(SCParametricMeshComponent)
class ENGINE_API SCParametricMeshComponent : public ParametricMeshComponent
{
	REFLECTION_BODY(SCParametricMeshComponent)

	explicit SCParametricMeshComponent(ObjectPtr<StaticMesh> InitMesh, int Iteration = 500);

	FORCEINLINE virtual FVector Sample(double U, double V) const override;

	// Use mesh vertex normal
	virtual FVector SampleNormal(double u, double v) const override;

	FORCEINLINE virtual bool ValidUV(double U, double V) const override;

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const override;

	virtual FVector2 Projection(const FVector& Point) const override;

protected:

	virtual void Remesh() override;

	struct UVMappingSampleResult
	{
		bool Valid;
		int TriangleIndex;
		double u, v;
		FVector Position;
	};
	UVMappingSampleResult SampleHit(double U, double V) const;

	SCParametricMeshComponent() = default;
	// UV Mesh, store the UV information of vertices
	MatrixX3d UVMesh;
	MatrixX3d Vertices;
	MatrixX3i Indices;

	// BVH tree which store the UV mesh, used to fast sample UV
	using BVHNode = bvh::v2::Node<double, 3>;
	bvh::v2::Bvh<BVHNode> BVHUVMesh;

	ObjectPtr<StaticMesh> OriginalMesh;

	// Triangle structure
	TArray<bvh::v2::PrecomputedTri<double>> Triangles;
};
