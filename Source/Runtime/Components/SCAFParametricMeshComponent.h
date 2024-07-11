//
// Created by MarvelLi on 2024/3/17.
//

#pragma once
#include "ParametricAlgorithmComponent.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/tri.h>
#include "Components/ParametricMeshComponent.h"

/**
 * This component support a bijective mapping from 3D position to UV coordinate
 * Require the original mesh is an open mesh
 * Support sample a UV cordiante and return the 3D position (UV coordinate maybe invalid)
 */
MCLASS(SCAFParametricMeshComponent)
class ENGINE_API SCAFParametricMeshComponent : public ParametricAlgorithmComponent
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

	SCAFParametricMeshComponent(const ObjectPtr<StaticMesh>& InDisplayMesh, const ObjectPtr<StaticMesh>& InPMesh);

	virtual UVMappingSampleResult SampleHit(double U, double V) const override;

	virtual TArray<FVector> GeodicShortestPath(const FVector& Start, const FVector& End) const override;


private:
	// BVH tree which store the UV mesh, used to fast sample UV
	bvh::v2::Bvh<BVHNode> BVHUVMesh;

	// Triangle structure
	TArray<bvh::v2::PrecomputedTri<double>> Triangles;
};
