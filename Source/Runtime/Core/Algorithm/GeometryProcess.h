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

	/**
	 * Given a mesh, and a time series of transformation, return the swept volume of the mesh motion.
	 * @param Profile The mesh performing motion
	 * @param Path The time series of transformation
	 * @param Steps The number of steps to sample the swept volume, -1 means auto, when greater than given time steps, the algorithm will interpolate the path
	 * @param GridSize The grid size of the longest edge of the bounding box of the swept volume
	 * @return The swept volume of the mesh of the motion
	 */
	ENGINE_API ObjectPtr<StaticMesh> SweptVolume(const ObjectPtr<StaticMesh>& Profile, const TArray<FTransform>& Path, int Steps = -1, int GridSize = 100);

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

	/**
	 * Given a mesh, and a thickness, return a new mesh which is the solidified version of the input mesh
	 * When Thickness is positive, the solidified mesh will thicken inside the mesh.
	 * @see Blender solidify modifier
	 * @warning This function works by offset mesh with vertex normal direction. May cause self-intersection well when thickness is too large.
	 * @param Mesh The input mesh
	 * @param Thickness The thickness of the solidified mesh
	 * @return The solidified mesh
	 */
	ENGINE_API ObjectPtr<StaticMesh> SolidifyMesh(const ObjectPtr<StaticMesh>& Mesh, double Thickness);
	ENGINE_API ObjectPtr<StaticMesh> SolidifyMeshEven(const ObjectPtr<StaticMesh>& Mesh, double Thickness); // Solidfy with half thickness on both side


};