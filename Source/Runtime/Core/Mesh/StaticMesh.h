//
// Created by Mayn on 8/25/2023.
//

#pragma once
#include "CoreMinimal.h"
#include "Math/Box.h"
#include "Math/FTransform.h"

// Material property update
DECLARE_MULTICAST_DELEGATE(FOnMaterialUpdate);
// Change to a new material
DECLARE_MULTICAST_DELEGATE(FOnMaterialChange);

class Material;
/**
 * StaticMesh is a data container of geometry, which stored in model space.
 */
MCLASS(StaticMesh)
class StaticMesh : public Object
{
	REFLECTION_BODY(StaticMesh)
public:
	StaticMesh();
	StaticMesh(const MatrixX3d& InVerM, const MatrixX3i& InTriM);
	StaticMesh(MatrixX3d&& InVerM, MatrixX3i&& InTriM);
	StaticMesh(const TArray<Vector3d>& verList, const TArray<Vector3i>& triList);
	StaticMesh(const TArray<double>& verList, const TArray<int>& triList);

	StaticMesh(StaticMesh&& Other) noexcept;
	StaticMesh(const StaticMesh& Other);

	// This is model space data
	MatrixX3d  verM; // Store vertices in a matrix (n,3)
	MatrixX3i  triM; // Store triangles in a matrix (m,3)
	//! DEPRECATED, only used in Libigl
	MatrixX3d  norM; // Store per triangle normal or vertex normal in a matrix (m,3) or (n,3).
	//! DEPRECATED, only used in Libigl
	MatrixX3d  colM; // Store per triangle color or vertex color in a matrix (m,3) or (n,3).

	MatrixX3d  VertexNormal; // Store per vertex normal in a matrix (n,3), this is mandatory for rendering

	ObjectPtr<StaticMesh> operator = (ObjectPtr<StaticMesh> Other);
	StaticMesh& operator = (StaticMesh&& Other) noexcept;

	FORCEINLINE Material* GetMaterial() const;
	FORCEINLINE void SetMaterial(ObjectPtr<Material> InMaterial);
	FORCEINLINE FVector GetVertex(int Index) const;
	FORCEINLINE FVector GetVertexNormal(int Index) const;
	FORCEINLINE Vector3i GetTriangle(int Index) const;
	FORCEINLINE FVector GetTriangleCenter(int Index) const;
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
	 * Set the color of the mesh
	 * @param Color Linear color of the mesh
	 */
	void SetColor(const FColor& Color);

	/**
	 * Explict calculate the normal of the mesh
	 * Should called when the mesh is modified
	 */
	void CalcNormal();

	/**
	 * Check if the mesh has normal and satisfy the normal option
	 * @return if the normal is valid
	 */
	bool CheckNormalValid() const;

	/**
	 * Reverse the normal of the mesh
	 * By reverse indices in the triangles order
	 */
	void ReverseNormal();

	/**
	 * Delete one vertex of the mesh
	 * @param VertexIndex Index of the vertex to be deleted
	 */
	void RemoveVertex(int VertexIndex);

	/**
	 * Delete multiple vertices of the mesh
	 * @param VertexIndices Indices of the vertices to be deleted
	 */
	void RemoveVertices(const TArray<int>& VertexIndices);

	/**
	 * Delete one face of the mesh
	 * @param FaceIndex Index of the face to be deleted
	 */
	void RemoveFace(int FaceIndex);

	/**
	 * Delete multiple faces of the mesh
	 * @param FaceIndices Indices of the faces to be deleted
	 */
	void RemoveFaces(const TArray<int>& FaceIndices);

	/**
	 * Submesh the mesh with given face indices
	 * @param FaceIndices Indices of the faces to be submeshed
	 * @return Submesh containing the given faces
	 */
	ObjectPtr<StaticMesh> SubMesh(const TArray<int>& FaceIndices);

	/**
	 * Remove the isolated vertices of the mesh(not used in any face)
	 * @return this
	 */
	StaticMesh& RemoveIsolatedVertices();

	/**
	 * Detect if the mesh has isolated vertices
	 * @return if the mesh has isolated vertices
	 */
	bool HasIsolatedVertices() const;

	/**
	 * Clear all the data of the mesh, make it empty
	 */
	void Clear();

	/**
	 * If the mesh is empty
	 */
	bool IsEmpty() const;

	/**
	 * Transform the mesh with a transform matrix
	 * @param TransformMatrix transform matrix
	 */
	void TransformMesh(const Matrix4d& TransformMatrix);
	void TransformMesh(const FTransform& Transform);

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
	 * @return this
	 */
	StaticMesh& Normlize();

	/**
	 * Get the Axis alignied bounding box of the mesh
	 * @return bounding box of the mesh
	 */
	FORCEINLINE Math::FBox GetBoundingBox() const;

	/**
	 * Save the mesh to a obj file
	 * @param FileName file name of the obj file
	 */
	void SaveOBJ(String FileName) const;

	static ObjectPtr<StaticMesh> LoadFromObj(const Path& FileName);

protected:
	MPROPERTY()
	ObjectPtr<Material> MaterialData; // Material of the mesh

	FBox BoundingBox; // Bounding box of the mesh
	FORCEINLINE void UpdateBoundingBox();
	FORCEINLINE void OnGeometryUpdate();
};

FORCEINLINE Material* StaticMesh::GetMaterial() const
{
	return MaterialData.get();
}

FORCEINLINE void StaticMesh::SetMaterial(ObjectPtr<Material> InMaterial)
{
	MaterialData = std::move(InMaterial);
}

FORCEINLINE FVector StaticMesh::GetVertex(int Index) const
{
	ASSERT(Index < verM.rows());
	return verM.row(Index);
}

FORCEINLINE FVector StaticMesh::GetVertexNormal(int Index) const
{
	ASSERT(Index < VertexNormal.rows());
	return VertexNormal.row(Index);
}

FORCEINLINE Vector3i StaticMesh::GetTriangle(int Index) const
{
	ASSERT(Index < triM.rows());
	return triM.row(Index);
}

FORCEINLINE FVector StaticMesh::GetTriangleCenter(int Index) const
{
	ASSERT(Index < triM.rows());
	return (verM.row(triM(Index, 0)) + verM.row(triM(Index, 1)) + verM.row(triM(Index, 2))) / 3.;
}

FORCEINLINE Math::FBox StaticMesh::GetBoundingBox() const
{
	return BoundingBox;
}

FORCEINLINE void StaticMesh::OnGeometryUpdate()
{
	ASSERTMSG(triM.maxCoeff() < verM.rows(), "Invalid triangle index");
	UpdateBoundingBox();
	CalcNormal();
}

FORCEINLINE void StaticMesh::UpdateBoundingBox()
{
	BoundingBox = Math::FBox(verM);
}