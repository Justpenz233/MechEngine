//
// Created by Mayn on 8/25/2023.
//

#pragma once
#include "CoreMinimal.h"
#include "Math/Box.h"
#include "Object/Object.h"

enum MeshNormalOption
{
	PerFaceNormal,    // Automatically calculated
	PerVertexNormal,  // Automatically calculated
	PerCornerNormal,  // Automatically calculated
	UserDefinedNormal // User defined
};

/**
 * StaticMesh is a data container of geometry, which stored in model space.
 */
class StaticMesh : public Object
{
protected:
	FBox BoundingBox; // Bounding box of the mesh
	MeshNormalOption   NormalOption = PerCornerNormal;   // Normal option
	void UpdateBoundingBox();
public:
	StaticMesh()= default;
	StaticMesh(const MatrixX3d& InVerM, const MatrixX3i& InTriM);
	StaticMesh(MatrixX3d&& InVerM, MatrixX3i&& InTriM);
	StaticMesh(const TArray<Vector3d>& verList, const TArray<Vector3i>& triList);
	StaticMesh(const TArray<double>& verList, const TArray<int>& triList);

	StaticMesh(StaticMesh&& Other) noexcept;
	StaticMesh(const StaticMesh& Other);

	// This is model space data
	MatrixX3d  verM; // Store vertices in a matrix (n,3)
	MatrixX3i  triM; // Store triangles in a matrix (m,3)
	MatrixX3d  norM; // Store per triangle normal in a matrix (m,3)
	MatrixX3d  colM;

	ObjectPtr<StaticMesh> operator = (std::shared_ptr<StaticMesh> Other);
	StaticMesh& operator = (StaticMesh&& Other) noexcept;

	/**
	 * https://ieeexplore.ieee.org/document/958278 , to calc signed volume of a tetrahedron with Origin as the fourth vertex
	 * @return volume of this mesh
	 */
	double CalcVolume() const;

	/**
	* Get the vertex number of the mesh
	*/
	int GetVertexNum() const;

	/**
	 * Get the face number of the mesh
	 */
	int GetFaceNum() const;

	/**
	 * Transform the mesh with a transform matrix
	 * @param transMat transform matrix
	 */
	void TransformMesh(const Matrix4d& transMat);


	void SetColor(RowVector3d Color);
	void SetupNormal(const MeshNormalOption& Option);
	void CalcNormal();
	bool CheckNormalValid() const;
	void ReverseNormal();

	/**
	 * Clear all the data of the mesh, make it empty
	 */
	void Clear();

	/**
	 * If the mesh is empty
	 */
	bool IsEmpty() const;

	// TODO: Will destroy closure of the mesh
	void ClearDegenerate();

	void SaveOBJ(String FileName);

	void Translate(const FVector& Translation);
	void Rotate(const FQuat& Rotation);
	void RotateEuler(const FVector& RotationEuler);
	void Scale(const FVector& InScale);
	void Scale(double InScale);

	/**
	 * Check if the mesh is self intersect
	 * @return if the mesh is self intersect
	 */
	bool IsSelfIntersect() const;

	/**
	 * Fille all the holes in the mesh, the process will be slow
	 * @return false if the mesh does not have holes
	 */
	bool FillHoles();

	/**
	 * Explicit smooth the mesh cotan Laplacian
	 * @param Iteration iterations of smoothing
	 * @param UseUniform use uniform Laplacian or cotan Laplacian
	 */
	void SmoothMesh(int Iteration = 5, bool UseUniform = false);

	/**
	 * Normize the mesh, make the bounding box of the mesh to be (0,0,0) and size of bounding box to be 1
	 */
	void Normlize();

	FORCEINLINE Math::FBox GetBoundingBox() const
	{
		return BoundingBox;
	}

	static ObjectPtr<StaticMesh> LoadFromObj(const Path& FileName);
};
