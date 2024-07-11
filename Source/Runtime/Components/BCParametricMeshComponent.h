//
// Created by MarvelLi on 2024/6/26.
//

#pragma once
#include "ParametricAlgorithmComponent.h"
#include "Core/CoreMinimal.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/tri.h>

/***
 * Box border Conformal Parametric Mesh Component
 *
 */
MCLASS(BCParametricMeshComponent)
class ENGINE_API BCParametricMeshComponent : public ParametricAlgorithmComponent
{
	REFLECTION_BODY(BCParametricMeshComponent)
public:
	explicit BCParametricMeshComponent(ObjectPtr<StaticMesh> InDisplayMesh, ObjectPtr<StaticMesh> InPMesh);

protected:

	virtual UVMappingSampleResult SampleHit(double U, double V) const override;

	BCParametricMeshComponent() = default;
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