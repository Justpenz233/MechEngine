//
// Created by Mayn on 8/25/2023.
//

#pragma once
#include "CoreMinimal.h"
#include "Math/Box.h"

// Material property update
DECLARE_MULTICAST_DELEGATE(FOnMaterialUpdate);
// Change to a new material
DECLARE_MULTICAST_DELEGATE(FOnMaterialChange);

DECLARE_MULTICAST_DELEGATE(FOnGeometryUpdate);

class Material;
/**
 * StaticMesh is a data container of geometry, which stored in model space.
 */
MCLASS(StaticMesh)
class ENGINE_API StaticMesh : public Object
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

	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

	// This is model space data
	MatrixX3d  verM; // Store vertices in a matrix (n,3)
	MatrixX3i  triM; // Store triangles in a matrix (m,3)

	MatrixX3d VertexNormal; // Store per vertex normal in a matrix (V, 3)
	MatrixX3d CornerNormal; // Store per corner normal in a matrix (F * 3, 3), per face have 3 corner normal.

	StaticMesh& operator = (const StaticMesh& Other) noexcept;
	StaticMesh& operator = (StaticMesh&& Other) noexcept;

	FORCEINLINE Material* GetMaterial() const;
	FORCEINLINE ObjectPtr<Material> GetMaterialAsset() const;
	FORCEINLINE void SetMaterial(ObjectPtr<Material> InMaterial);
	FORCEINLINE const MatrixX3d& GetVertices() const;
	FORCEINLINE FVector GetVertex(int Index) const;
	FORCEINLINE FVector GetVertexNormal(int Index) const;
	FORCEINLINE const MatrixX3i& GetTriangles() const;
	FORCEINLINE Vector3i GetTriangle(int Index) const;
	FORCEINLINE FVector GetTriangleCenter(int Index) const;

	/**
	 * Set the geometry of the mesh
	 * @param InVerM InVerM vertices matrix
	 * @param InTriM InTriM triangles matrix
	 * @return this
	 */
	StaticMesh* SetGeometry(const MatrixX3d& InVerM, const MatrixX3i& InTriM);
	StaticMesh* SetGeometry(const MatrixX3d& InVerM);
	StaticMesh* SetGeometry(const MatrixX3i& InTriM);

	/**
	 * If the mesh has UV, which is UV != empty
	 * @return if the mesh has UV
	 */
	bool HasUV() const;

	/**
	 * If the mesh has valid UV, which is UV != empty and UV.rows() == vertex number
	 * @return if the mesh has valid UV
	 */
	bool HasValidUV() const;

	/**
	 * Set the UV of the mesh
	 * @param InUV UV matrix
	 * @return this
	 */
	StaticMesh* SetUV(MatrixX2d&& InUV);
	StaticMesh* SetUV(const MatrixX2d& InUV);
	StaticMesh* SetUV(int VertexIndex, const FVector2& InUV);

	/**
	 * Get the UV of the mesh
	 * @return UV matrix
	 */
	FORCEINLINE MatrixX2d GetUV() const { return UV; }
	FORCEINLINE FVector2 GetUV(int VertexIndex) const { return UV.row(VertexIndex); }

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
	 * Get the edge number of the mesh
	 * @return edge number of the mesh
	 */
	int GetEdgeNum() const;

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
	StaticMesh* ReverseNormal();

	/**
	 * Delete one vertex of the mesh
	 * @param VertexIndex Index of the vertex to be deleted
	 */
	StaticMesh* RemoveVertex(int VertexIndex);

	/**
	 * Delete multiple vertices of the mesh
	 * @param VertexIndices Indices of the vertices to be deleted
	 */
	StaticMesh* RemoveVertices(const TArray<int>& VertexIndices);

	/**
	 * Delete one face of the mesh
	 * @param FaceIndex Index of the face to be deleted
	 */
	StaticMesh* RemoveFace(int FaceIndex);

	/**
	 * Delete multiple faces of the mesh
	 * @param FaceIndices Indices of the faces to be deleted
	 */
	StaticMesh* RemoveFaces(const TArray<int>& FaceIndices);

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
	StaticMesh* RemoveIsolatedVertices();

	/**
	 * Detect if the mesh has isolated vertices
	 * @return if the mesh has isolated vertices
	 */
	bool HasIsolatedVertices() const;

	/**
	 * Clear all the data of the mesh, make it empty
	 */
	StaticMesh* Clear();

	/**
	 * If the mesh is empty
	 */
	bool IsEmpty() const;

	/**
	 * Transform the mesh with a transform matrix
	 * @param TransformMatrix transform matrix
	 */
	StaticMesh* TransformMesh(const Matrix4d& TransformMatrix);
	StaticMesh* TransformMesh(const FTransform& Transform);

	StaticMesh* Translate(const FVector& Translation);
	StaticMesh* Rotate(const FQuat& Rotation);
	StaticMesh* RotateEuler(const FVector& RotationEuler);
	StaticMesh* Scale(const FVector& InScale);
	StaticMesh* Scale(double InScale);

	/**
	 * Scale the mesh according to the bounding box center
	 * @param InScale scale factor
	 *
	 */
	StaticMesh* ScaleByBoundingBoxCenter(const double& InScale) {return ScaleByBoundingBoxCenter(FVector::Constant(InScale));}
	StaticMesh* ScaleByBoundingBoxCenter(const FVector& InScale);

	/**
	 * Offset the mesh along the vertex normal direction by a distance. A.K.A. shrink the mesh
	 * @param Distance offset distance
	 * @return offseted mesh
	 */
	StaticMesh* OffsetVertex(const double& Distance);

	/**
	 * Check if the mesh is self intersect
	 * @return if the mesh is self intersect
	 */
	bool IsSelfIntersect() const;

	/**
	 * Get the boundary vertices of the mesh
	 * @return boundary vertices indices, will be empty if the mesh does not have boundary
	 */
	TArray<int> BoundaryVertices() const;

	/**
	 * Get the boundary triangles of the mesh
	 * @return boundary triangles indices, will be empty if the mesh does not have boundary
	 */
	TArray<int> BoundaryTriangles() const;

	/**
	 * Check if the mesh is closed, i.e. the mesh is homeomorphic to a sphere
	 * @return true, if the mesh is closed
	 */
	FORCEINLINE bool IsClosed() const;

	/**
	 * Fille all the holes in the mesh, the process will be slow
	 * @return false if the mesh does not have holes
	 */
	StaticMesh* FillHoles();

	/**
	 * Explicit smooth the mesh cotan Laplacian
	 * @param Iteration iterations of smoothing
	 * @param UseUniform use uniform Laplacian or cotan Laplacian
	 */
	StaticMesh* SmoothMesh(int Iteration = 5, bool UseUniform = false);

	/**
	 * Clean the mesh, remove the isolated or NAN vertices
	 * @return this
	 */
	StaticMesh* Clean();

	/**
	 * Normize the mesh, make the bounding box of the mesh to be (0,0,0) and size of bounding box to be 1
	 * @return this
	 */
	StaticMesh* Normalize();
	ObjectPtr<StaticMesh> Normalized();
	/**
	 * Get the Axis alignied bounding box of the mesh
	 * @return bounding box of the mesh
	 */
	FORCEINLINE Math::FBox GetBoundingBox() const;

	/**
	 * Save the mesh to a obj file
	 * @param FileName file name of the obj file
	 */
	void SaveOBJ(const Path& FileName) const;


	static ObjectPtr<StaticMesh> LoadObj(const Path& FileName);

	/**
	 * Called after geometry data updated.
	 * Will recalculate the bounding box and normal of the mesh, and broadcast the OnGeometryUpdateDelegate
	 */
	FORCEINLINE void OnGeometryUpdate();

	/**
	 * Update the bounding box of the mesh.
	 * Will be automatically called after geometry data updated.
	 */
	FORCEINLINE void UpdateBoundingBox();

	FORCEINLINE FOnGeometryUpdate GetOnGeometryUpdateDelegate();


	/**
	 * Get the boundary vertices of the mesh
	 * @return boundary vertices indices, will be empty if the mesh does not have boundary
	 */
	TArray<TArray<int>> GetBoundaryVertices() const;

	/**
	 * Get the Genus of the mesh
	 * @return Genus of the mesh
	 */
	int GetGenus();

protected:
	MatrixX2d UV; // UV matrix of the mesh, each row is a UV coordinate of a vertex

	MPROPERTY()
	ObjectPtr<Material> MaterialData; // Material of the mesh

	MPROPERTY()
	double CornelThresholdDegree = 20; // Threshold degree to detect the corner normal

	FOnGeometryUpdate OnGeometryUpdateDelegate;

	FBox BoundingBox; // Bounding box of the mesh
};

FORCEINLINE Material* StaticMesh::GetMaterial() const
{
	return MaterialData.get();
}

FORCEINLINE ObjectPtr<Material> StaticMesh::GetMaterialAsset() const
{
	return MaterialData;
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

FORCEINLINE const MatrixX3d& StaticMesh::GetVertices() const
{
	return verM;
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

FORCEINLINE const MatrixX3i& StaticMesh::GetTriangles() const
{
	return triM;
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
	if(triM.rows() == 0 || verM.rows() == 0)
	{
		LOG_ERROR("StaticMesh constructed with null geometry data");
		return;
	}
	if(triM.maxCoeff() >= verM.rows())
	{
		LOG_ERROR("Invalid triangle index, max triangle index: {}, vertex number: {}", triM.maxCoeff(), verM.rows());
		return;
	}
	UpdateBoundingBox();
	CalcNormal();
	OnGeometryUpdateDelegate.Broadcast();
}

FORCEINLINE void StaticMesh::UpdateBoundingBox()
{
	BoundingBox = Math::FBox(verM);
}

FORCEINLINE FOnGeometryUpdate StaticMesh::GetOnGeometryUpdateDelegate()
{
	return OnGeometryUpdateDelegate;
}

FORCEINLINE bool StaticMesh::IsClosed() const
{
	return BoundaryVertices().empty();
}