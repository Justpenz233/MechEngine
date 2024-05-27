//
// Created by MarvelLi on 2024/1/30.
//


#pragma once
#include "CoreMinimal.h"

class StaticMesh;

namespace MechEngine::Algorithm::GeometryProcess
{
	// Fill the smallest hole in the mesh, return false if there is no hole
	ENGINE_API bool FillSmallestHole(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles);

	// Fill all holes in the mesh, return false if there is no hole
	ENGINE_API bool FillAllHoles(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles);

	// Explicit smooth the mesh cotan Laplacian
	ENGINE_API void SmoothMesh(Eigen::MatrixX3d& Vertices, Eigen::MatrixX3i& Triangles, int Iteration = 5, bool UseUniformLaplacian = false);

	ENGINE_API ObjectPtr<StaticMesh> SweptVolume(const ObjectPtr<StaticMesh>& Profile, const TArray<FTransform>& Path, int Steps = -1);

	/**
	 * \brief Give a mesh which is constructed by many disconnected components, split the mesh into components(by adjacency graph of edges and vertices)
	 * \param Mesh Under processing mesh
	 * \return Array of divided mesh components
	 */
	ENGINE_API TArray<ObjectPtr<StaticMesh>> DivideMeshIntoComponents(const ObjectPtr<StaticMesh>& Mesh);

	/**
	 * \brief Give a mesh which is constructed by many disconnected components, count the number of components in the mesh
	 * \return Number of components
	 */
	ENGINE_API int MeshComponentsCount(const ObjectPtr<StaticMesh>& Mesh);

	/**
	 * Given a 3D position and a SampleUV function, project the 3D position to 2D UV space
	 * This algorithm will enumerate a grid(10*10) from [0., 1.]*[0., 1.] space as inital guess, and from the inital guess runing a LevenbergMarquardt to optimize the UV position
	 * @param Pos 3D position
	 * @param SampleFunction SampleUV function
	 * @return 2D UV position
	 */
	ENGINE_API FVector2 Projection(const FVector& Pos, TFunction<FVector(const FVector2&)> SampleFunction);
};