//
// Created by MarvelLi on 2024/6/6.
//

#pragma once
#include "ParametricAlgorithmComponent.h"
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
class ENGINE_API SCParametricMeshComponent : public ParametricAlgorithmComponent
{
	REFLECTION_BODY(SCParametricMeshComponent)

	explicit SCParametricMeshComponent(const ObjectPtr<StaticMesh>& InDisplayMesh, const ObjectPtr<StaticMesh>& InPMesh, int Iteration = 500);

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const override;

protected:

	UVMappingSampleResult SampleHit(double U, double V) const;

	SCParametricMeshComponent() = default;
	// UV Mesh, store the UV information of vertices
	MatrixX3d UVMesh;
	MatrixX3d Vertices;
	MatrixX3i Indices;

	// BVH tree which store the UV mesh, used to fast sample UV
	using BVHNode = bvh::v2::Node<double, 3>;
	bvh::v2::Bvh<BVHNode> BVHUVMesh;

	// Triangle structure
	TArray<bvh::v2::PrecomputedTri<double>> Triangles;
};
