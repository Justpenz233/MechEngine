//
// Created by Mayn on 8/25/2023.
//

#include <fstream>
#include "Log/Log.h"
#include "StaticMesh.h"
#include "Math/LinearAlgebra.h"
#include "igl/per_face_normals.h"
#include "igl/per_vertex_normals.h"
#include "igl/per_corner_normals.h"
#include "igl/readOBJ.h"
#include "igl/fast_find_self_intersections.h"
#include "Algorithm/GeometryProcess.h"
#include "Materials/Material.h"
#include "Misc/Path.h"

StaticMesh::StaticMesh()
{
	MaterialData = Material::DefaultMaterial();
}

StaticMesh::StaticMesh(const MatrixX3d& InVerM, const MatrixX3i& InTriM)
{
	verM = InVerM;
	triM = InTriM;
	MaterialData = Material::DefaultMaterial();
	OnGeometryUpdate();
}

StaticMesh::StaticMesh(MatrixX3d&& InVerM, MatrixX3i&& InTriM)
{
	verM = std::move(InVerM);
	triM = std::move(InTriM);
	MaterialData = Material::DefaultMaterial();
	OnGeometryUpdate();
}

StaticMesh::StaticMesh(const TArray<Vector3d>& verList, const TArray<Vector3i>& triList)
{
	verM.resize(verList.size(), 3);
	for (int i = 0; i < verList.size(); i++)
		verM.row(i) = verList[i];

	triM.resize(triList.size(), 3);
	for (int i = 0; i < triList.size(); i++)
		triM.row(i) = triList[i];
	MaterialData = Material::DefaultMaterial();
	OnGeometryUpdate();
}

StaticMesh::StaticMesh(const TArray<double>& verList, const TArray<int>& triList)
{
	verM.resize(verList.size() / 3, 3);
	triM.resize(triList.size() / 3, 3);
	for (int i = 0;i < verList.size();i += 3)
		verM.row(i / 3) = RowVector3d(verList[i], verList[i + 1], verList[i + 2]);
	for (int i = 0; i < triList.size(); i += 3)
		triM.row(i / 3) = Eigen::RowVector3i(triList[i], triList[i + 1], triList[i + 2]);
	MaterialData = Material::DefaultMaterial();
	OnGeometryUpdate();
}

StaticMesh::StaticMesh(StaticMesh&& Other) noexcept
{
	verM = std::move(Other.verM);
	triM = std::move(Other.triM);
	VertexNormal = std::move(Other.VertexNormal);
	CornerNormal = std::move(Other.CornerNormal);
	BoundingBox = Other.BoundingBox;
	MaterialData = std::move(Other.MaterialData);
	OnGeometryUpdateDelegate.Broadcast();
}

StaticMesh::StaticMesh(const StaticMesh& Other)
 : Object(Other) {
	verM = Other.verM;
	triM = Other.triM;
	VertexNormal = Other.VertexNormal;
	CornerNormal = Other.CornerNormal;
	BoundingBox = Other.BoundingBox;
	MaterialData = NewObject<Material>(*Other.MaterialData);
	OnGeometryUpdateDelegate.Broadcast();
}
void StaticMesh::PostEdit(Reflection::FieldAccessor& Field)
{
	Object::PostEdit(Field);
	if (Field.getFieldName() == NAME(CornelThresholdDegree))
	{
		OnGeometryUpdate();
		GetPostEditDelegate().Broadcast(Field);
	}
}

StaticMesh& StaticMesh::operator=(const StaticMesh& Other) noexcept
{
	verM = Other.verM;
	triM = Other.triM;
	VertexNormal = Other.VertexNormal;
	CornerNormal = Other.CornerNormal;
	BoundingBox = Other.BoundingBox;
	MaterialData = NewObject<Material>(*Other.MaterialData);
	OnGeometryUpdateDelegate.Broadcast();
	return *this;
}

StaticMesh& StaticMesh::operator=(StaticMesh&& Other) noexcept
{
	verM = std::move(Other.verM);
	triM = std::move(Other.triM);
	VertexNormal = std::move(Other.VertexNormal);
	CornerNormal = std::move(Other.CornerNormal);
	BoundingBox = Other.BoundingBox;
	MaterialData = std::move(Other.MaterialData);
	OnGeometryUpdateDelegate.Broadcast();
	return *this;
}

StaticMesh* StaticMesh::TransformMesh(const Matrix4d& TransformMatrix)
{
	if (TransformMatrix == Matrix4d::Identity())
		return this;

	ParallelFor(verM.rows(), [this, &TransformMatrix](int i) {
		Vector4d NewPos = TransformMatrix * (verM.row(i).homogeneous()).transpose();
		verM.row(i) = NewPos.head(3); }, 1e7);

	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::TransformMesh(const FTransform& Transform)
{
	if (Transform == FTransform::Identity())
		return this;

	ParallelFor(verM.rows(), [&](int i) {
		Vector3d NewPos = Transform * FVector(verM.row(i));
		verM.row(i) = NewPos; }, 1e7);

	OnGeometryUpdate();
	return this;
}

void StaticMesh::SaveOBJ(const Path& FileName) const
{
	// No extension add .obj
	Path FixedFileName = FileName;
	if (!FileName.has_extension())
		FixedFileName += ".obj";
	if (FixedFileName.Existing())
		LOG_WARNING("File {} already exists, will overwrite", FixedFileName.string());
	
	std::fstream ofile;
	ofile.open(FileName, std::ios::out);
	if (!ofile)
	{
		LOG_ERROR("Open File {} fail", FileName.string());
		return;
	}

	for (int i = 0; i < verM.rows(); i++) {
		ofile << "v " << verM(i, 0) << " " << verM(i, 1) << " " << verM(i, 2) << std::endl;
	}
	for (int i = 0; i < triM.rows(); i++) {
		ofile << "f " << triM(i, 0) + 1 << " " << triM(i, 1) + 1 << " " << triM(i, 2) + 1 << std::endl;
	}

	ofile.close();
}

StaticMesh* StaticMesh::Translate(const FVector& Translation)
{
	ParallelFor(verM.rows(), [this, Translation](int i){
		verM.row(i) += Translation;
	});
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::Rotate(const FQuat& Rotation)
{
	ParallelFor(verM.rows(), [this, Rotation](int i){
		verM.row(i) = verM.row(i) * Rotation.matrix();
	});
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::RotateEuler(const FVector& RotationEuler)
{
	TransformMesh(MMath::RotationMatrix4FromEulerXYZ(RotationEuler));
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::Scale(const FVector& InScale)
{
	ParallelFor(verM.rows(), [this, InScale](int i) {
		verM(i, 0) *= InScale[0];
		verM(i, 1) *= InScale[1];
		verM(i, 2) *= InScale[2];
	});
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::Scale(double InScale)
{
	Scale({ InScale, InScale, InScale });
	return this;
}

StaticMesh* StaticMesh::ScaleByBoundingBoxCenter(const FVector& InScale)
{
	auto Center = GetBoundingBox().GetCenter();
	ParallelFor(verM.rows(), [this, InScale, Center](int i) {
		verM.row(i) = Center + (FVector(verM.row(i)) - Center).cwiseProduct(InScale);
	});
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::OffsetVertex(const double& Distance)
{
	ParallelFor(verM.rows(), [this, Distance](int i) {
		verM.row(i) += Distance * VertexNormal.row(i);
	});
	OnGeometryUpdate();
	return this;
}

bool StaticMesh::IsSelfIntersect() const
{
	MatrixXd IF, EV, EE;
	Eigen::VectorXi EI;
	igl::fast_find_self_intersections(verM, triM, true, true, IF, EV, EE, EI);
	return IF.rows() > 0;
}

StaticMesh* StaticMesh::FillHoles()
{
	auto Result = Algorithm::GeometryProcess::FillAllHoles(verM, triM);
	if (Result) OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::SmoothMesh(int Iteration, bool UseUniform)
{
	Algorithm::GeometryProcess::SmoothMesh(verM, triM, Iteration, UseUniform);
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::Clean()
{
	TArray<int> ToRemoveVertex;
	for (int i = 0;i < verM.rows();i ++)
	{
		if(verM.row(i).hasNaN())
			ToRemoveVertex.push_back(i);
	}
	if(ToRemoveVertex.size() > 0)
		RemoveVertices(ToRemoveVertex);
	return this;
}

StaticMesh* StaticMesh::RemoveIsolatedVertices()
{
	THashSet<int> VertexSet;
	for (int i = 0; i < triM.rows(); i++)
	{
		VertexSet.insert(triM(i, 0));
		VertexSet.insert(triM(i, 1));
		VertexSet.insert(triM(i, 2));
	}
	if (VertexSet.size() == verM.rows())
		return this;

	THashMap<int, int> VertexMap;
	int Index = 0;
	for (int VertexIndex : VertexSet)
	{
		VertexMap[VertexIndex] = Index;
		Index++;
	}
	MatrixX3d NewVerM(VertexSet.size(), 3);
	for (auto& [VertexIndex, NewIndex] : VertexMap)
	{
		NewVerM.row(NewIndex) = verM.row(VertexIndex);
	}
	for (int i = 0; i < triM.rows(); i++)
	{
		triM(i, 0) = VertexMap[triM(i, 0)];
		triM(i, 1) = VertexMap[triM(i, 1)];
		triM(i, 2) = VertexMap[triM(i, 2)];
	}
	verM = NewVerM;
	OnGeometryUpdate();
	return this;
}

bool StaticMesh::HasIsolatedVertices() const
{
	THashSet<int> VertexSet;
	for (int i = 0; i < triM.rows(); i++)
	{
		VertexSet.insert(triM(i, 0));
		VertexSet.insert(triM(i, 1));
		VertexSet.insert(triM(i, 2));
	}
	return VertexSet.size() != verM.rows();
}

StaticMesh* StaticMesh::Normalize()
{
	auto Center = GetBoundingBox().GetCenter();
	auto Scale = 1. / GetBoundingBox().GetSize().maxCoeff();
	verM.rowwise() -= Center.transpose();
	verM *= Scale;

	OnGeometryUpdate();
	return this;
}
ObjectPtr<StaticMesh> StaticMesh::Normalized()
{
	Normalize();
	return GetThis<StaticMesh>();
}

ObjectPtr<StaticMesh> StaticMesh::LoadObj(const Path& FileName)
{
	if (FileName.extension() == ".obj" || FileName.extension() == ".OBJ")
	{
		MatrixX3d verM;
		MatrixX3i triM;
		Path	  FixedFileName = FileName;
		if (!FileName.has_parent_path())
		{
			if (exists((Path::EngineContentDir() / FileName)))
				FixedFileName = Path::EngineContentDir() / FileName;
			if (exists((Path::ProjectContentDir() / FileName)))
				FixedFileName = Path::ProjectContentDir() / FileName;
		}
		if (igl::readOBJ(FixedFileName.string(), verM, triM))
		{
			return NewObject<StaticMesh>(verM, triM);
		}
		LOG_ERROR("Fail to load mesh from file: {0}", FileName.string());
	}
	else
		LOG_ERROR("Extension of file is not .obj or .OBJ: {0}", FileName.string());
}

StaticMesh* StaticMesh::SetGeometry(const MatrixX3d& InVerM, const MatrixX3i& InTriM)
{
	verM = InVerM; triM = InTriM; OnGeometryUpdate();
	return this;
}
StaticMesh* StaticMesh::SetGeometry(const MatrixX3d& InVerM)
{
	verM = InVerM; OnGeometryUpdate();
	return this;
}
StaticMesh* StaticMesh::SetGeometry(const MatrixX3i& InTriM)
{
	triM = InTriM; OnGeometryUpdate();
	return this;
}

double StaticMesh::CalcVolume() const
{
	RowVector3d a, b, c, d;
	double vol = 0;
	for (int i = 0; i < triM.rows(); i++) {
		b   = verM.row(triM(i, 0));
		c   = verM.row(triM(i, 1));
		d   = verM.row(triM(i, 2));
		a   = RowVector3d(0, 0, 0);
		vol = vol - (a - d).dot((b - d).cross(c - d)) / 6.;
	}
	return vol;
}

int StaticMesh::GetVertexNum() const
{
	return verM.rows();
}

int StaticMesh::GetFaceNum() const
{
	return triM.rows();
}

void StaticMesh::CalcNormal()
{
	igl::per_vertex_normals(verM, triM, VertexNormal);
	igl::per_corner_normals(verM, triM, CornelThresholdDegree, CornerNormal);
}

bool StaticMesh::CheckNormalValid() const
{
	return VertexNormal.rows() == verM.rows() && CornerNormal.rows() == triM.rows() * 3;
}

StaticMesh* StaticMesh::ReverseNormal()
{
	for (int i = 0; i < triM.rows(); i++)
	{
		int tmp_y = triM(i, 1);
		int tmp_z = triM(i, 2);

		triM(i, 1) = tmp_z;
		triM(i, 2) = tmp_y;
	}
	OnGeometryUpdate();
	return this;
}

StaticMesh* StaticMesh::RemoveVertex(int VertexIndex)
{
	for (int i = 0;i < triM.rows();i ++)
	{
		if (triM(i, 0) == VertexIndex || triM(i, 1) == VertexIndex || triM(i, 2) == VertexIndex)
		{
			triM.row(i) = triM.row(triM.rows() - 1);
			triM.conservativeResize(triM.rows() - 1, 3);
			i--;
		}
	}
	RemoveIsolatedVertices();
	return this;
}

StaticMesh* StaticMesh::RemoveVertices(const TArray<int>& VertexIndices)
{
	THashSet<int> VertexSet;
	for (int VertexIndice : VertexIndices) VertexSet.insert(VertexIndice);
	for (int i = 0;i < triM.rows();i ++)
	{
		if (VertexSet.contains(triM(i, 0)) || VertexSet.contains(triM(i, 1)) || VertexSet.contains(triM(i, 2)))
		{
			triM.row(i) = triM.row(triM.rows() - 1);
			triM.conservativeResize(triM.rows() - 1, 3);
			i--;
		}
	}
	RemoveIsolatedVertices();
	return this;
}

StaticMesh* StaticMesh::RemoveFace(int FaceIndex)
{
	if (FaceIndex < 0 || FaceIndex >= triM.rows())
	{
		LOG_WARNING("Try to remove face with index out of range: {0}", FaceIndex);
	}
	MatrixX3i NewTriM(triM.rows() - 1, 3);
	NewTriM.topRows(FaceIndex) = triM.topRows(FaceIndex);
	NewTriM.bottomRows(triM.rows() - FaceIndex - 1) = triM.bottomRows(triM.rows() - FaceIndex - 1);
	RemoveIsolatedVertices();
	return this;
}

StaticMesh* StaticMesh::RemoveFaces(const TArray<int>& FaceIndices)
{
	THashSet<int> FaceSet;
	for (int FaceIndex : FaceIndices) FaceSet.insert(FaceIndex);
	MatrixX3i NewTriM(triM.rows() - FaceSet.size(), 3);
	int Index = 0;
	for (int i = 0; i < triM.rows(); i++)
	{
		if (FaceSet.contains(i)) continue;
		NewTriM.row(Index) = triM.row(i);
		Index++;
	}
	triM = NewTriM;
	RemoveIsolatedVertices();
	return this;
}

ObjectPtr<StaticMesh> StaticMesh::SubMesh(const TArray<int>& FaceIndices)
{
	MatrixX3d NewVerM;
	MatrixX3i NewTriM;
	THashSet<int> VertexSet;
	for (int FaceIndex : FaceIndices)
	{
		VertexSet.insert(triM(FaceIndex, 0));
		VertexSet.insert(triM(FaceIndex, 1));
		VertexSet.insert(triM(FaceIndex, 2));
	}
	THashMap<int, int> VertexMap;
	int Index = 0;
	for (int VertexIndex : VertexSet)
	{
		VertexMap[VertexIndex] = Index;
		Index++;
	}
	NewVerM.resize(VertexSet.size(), 3);
	for (auto& [VertexIndex, NewIndex] : VertexMap)
	{
		NewVerM.row(NewIndex) = verM.row(VertexIndex);
	}
	NewTriM.resize(FaceIndices.size(), 3);
	for (int i = 0; i < FaceIndices.size(); i++)
	{
		NewTriM(i, 0) = VertexMap[triM(FaceIndices[i], 0)];
		NewTriM(i, 1) = VertexMap[triM(FaceIndices[i], 1)];
		NewTriM(i, 2) = VertexMap[triM(FaceIndices[i], 2)];
	}
	return NewObject<StaticMesh>(NewVerM, NewTriM);
}

StaticMesh* StaticMesh::Clear()
{
	verM.resize(0, 3);
	triM.resize(0, 3);
	VertexNormal.resize(0, 3);
	CornerNormal.resize(0, 3);
	BoundingBox = Math::FBox();
	OnGeometryUpdateDelegate.Broadcast();
	return this;
}

bool StaticMesh::IsEmpty() const
{
	return verM.rows() < 3 || triM.rows() < 1;
}