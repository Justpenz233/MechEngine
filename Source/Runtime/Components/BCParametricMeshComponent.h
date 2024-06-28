//
// Created by MarvelLi on 2024/6/26.
//

#pragma once
#include "ParametricMeshComponent.h"
#include "Core/CoreMinimal.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/tri.h>

/***
 * Box border Conformal Parametric Mesh Component
 *
 */
MCLASS(BCParametricMeshComponent)
class ENGINE_API BCParametricMeshComponent : public ParametricMeshComponent
{
	REFLECTION_BODY(BCParametricMeshComponent)
public:
	explicit BCParametricMeshComponent(ObjectPtr<StaticMesh> InitMesh);

	FORCEINLINE virtual FVector Sample(double U, double V) const override;

	// Use mesh vertex normal
	virtual FVector SampleNormal(double u, double v) const override;

	FORCEINLINE virtual bool ValidUV(double U, double V) const override;
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

	BCParametricMeshComponent() = default;
	// UV Mesh, store the UV information of vertices
	MatrixX3d UVMesh;
	MatrixX3d Vertices;
	MatrixX3i Indices;
	ObjectPtr<StaticMesh> OriginalMesh;

	// BVH tree which store the UV mesh, used to fast sample UV
	using BVHNode = bvh::v2::Node<double, 3>;
	bvh::v2::Bvh<BVHNode> BVHUVMesh;

	// Triangle structure
	TArray<bvh::v2::PrecomputedTri<double>> Triangles;

};

FORCEINLINE FVector BCParametricMeshComponent::Sample(double U, double V) const
{
	NormlizeUV(U, V);
	return SampleHit(U, V).Position;
}

FORCEINLINE bool BCParametricMeshComponent::ValidUV(double U, double V) const
{
	NormlizeUV(U, V);
	auto Hit = SampleHit(U, V);
	return Hit.Valid;
}