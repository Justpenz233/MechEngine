//
// Created by Mayn on 8/25/2023.
//

#include <fstream>
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
	if (Other.colM.rows() != 0)
		colM = std::move(Other.colM);
	if (Other.norM.rows() != 0)
		norM = std::move(Other.norM);
	MaterialData = std::move(Other.MaterialData);
	OnGeometryUpdate();
}

StaticMesh::StaticMesh(const StaticMesh& Other)
{
	verM = Other.verM;
	triM = Other.triM;
	colM = Other.colM;
	norM = Other.norM;
	MaterialData = NewObject<Material>(*Other.MaterialData);
	OnGeometryUpdate();
}

ObjectPtr<StaticMesh> StaticMesh::operator=(std::shared_ptr<StaticMesh> Other)
{
	verM      = Other->verM;
	triM      = Other->triM;
	if (Other->colM.rows() != 0) colM = Other->colM.eval();
	if (Other->norM.rows() != 0) norM = Other->norM.eval();
	MaterialData = NewObject<Material>(*Other->MaterialData);
	OnGeometryUpdate();
	return Cast<StaticMesh>(GetThis());
}

StaticMesh& StaticMesh::operator=(StaticMesh&& Other) noexcept
{
	verM = std::move(Other.verM);
	triM = std::move(Other.triM);
	if (Other.colM.rows() != 0) colM = std::move(Other.colM);
	if (Other.norM.rows() != 0) norM = std::move(Other.norM);
	MaterialData = std::move(Other.MaterialData);
	OnGeometryUpdate();
	return *this;
}

void StaticMesh::TransformMesh(const Matrix4d& TransformMatrix)
{
	if (TransformMatrix == Matrix4d::Identity())
		return;

	ParallelFor(verM.rows(), [this, &TransformMatrix](int i) {
		Vector4d NewPos = TransformMatrix * (verM.row(i).homogeneous()).transpose();
		verM.row(i) = NewPos.head(3); }, 1e7);

	OnGeometryUpdate();
}
void StaticMesh::TransformMesh(const FTransform& Transform)
{
	if (Transform == FTransform::Identity())
		return;

	ParallelFor(verM.rows(), [&](int i) {
		Vector3d NewPos = Transform * FVector(verM.row(i));
		verM.row(i) = NewPos; }, 1e7);

	OnGeometryUpdate();
}

void StaticMesh::SaveOBJ(const Path& FileName) const
{
	SaveOBJ(FileName.string());
}

void StaticMesh::SaveOBJ(String FileName) const
{
	std::filesystem::path FilePath = FileName;
	if (FilePath.extension() != ".obj")
		FileName += ".obj";
	std::fstream ofile;
	ofile.open(FileName, std::ios::out);
	if (!ofile)
	{
		LOG_ERROR("Open File {} fail", FileName);
	}

	int vrows = verM.rows();
	for (int i = 0; i < vrows; i++) {
		ofile << "v " << verM(i, 0) << " " << verM(i, 1) << " " << verM(i, 2) << std::endl;
	}
	int frows = triM.rows();
	for (int i = 0; i < frows; i++) {
		ofile << "f " << triM(i, 0) + 1 << " " << triM(i, 1) + 1 << " " << triM(i, 2) + 1 << std::endl;
	}

	ofile.close();
}

void StaticMesh::Translate(const FVector& Translation)
{
	ParallelFor(verM.rows(), [this, Translation](int i){
		verM.row(i) += Translation;
	});
	OnGeometryUpdate();
}

void StaticMesh::Rotate(const FQuat& Rotation)
{
	ParallelFor(verM.rows(), [this, Rotation](int i){
		verM.row(i) = verM.row(i) * Rotation.matrix();
	});
	OnGeometryUpdate();
}

void StaticMesh::RotateEuler(const FVector& RotationEuler)
{
	TransformMesh(MMath::RotationMatrix4FromEulerXYZ(RotationEuler));
	OnGeometryUpdate();
}

void StaticMesh::Scale(const FVector& InScale)
{
	ParallelFor(verM.rows(), [this, InScale](int i) {
		verM(i, 0) *= InScale[0];
		verM(i, 1) *= InScale[1];
		verM(i, 2) *= InScale[2];
	});
	OnGeometryUpdate();
}

void StaticMesh::Scale(double InScale)
{
	Scale({InScale, InScale, InScale});
}

bool StaticMesh::IsSelfIntersect() const
{
	MatrixXd IF, EV, EE;
	Eigen::VectorXi EI;
	igl::fast_find_self_intersections(verM, triM, true, true, IF, EV, EE, EI);
	return IF.rows() > 0;
}

bool StaticMesh::FillHoles()
{
	auto Result = Algorithm::GeometryProcess::FillAllHoles(verM, triM);
	if (Result) OnGeometryUpdate();
	return Result;
}

void StaticMesh::SmoothMesh(int Iteration, bool UseUniform)
{
	Algorithm::GeometryProcess::SmoothMesh(verM, triM, Iteration, UseUniform);
	OnGeometryUpdate();
}

StaticMesh& StaticMesh::RemoveIsolatedVertices()
{
	THashSet<int> VertexSet;
	for (int i = 0; i < triM.rows(); i++)
	{
		VertexSet.insert(triM(i, 0));
		VertexSet.insert(triM(i, 1));
		VertexSet.insert(triM(i, 2));
	}
	if (VertexSet.size() == verM.rows())
		return *this;

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
	return *this;
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

StaticMesh& StaticMesh::Normlize()
{
	auto Center = GetBoundingBox().GetCenter();
	auto Scale = 1. / GetBoundingBox().GetSize().maxCoeff();
	verM.rowwise() -= Center.transpose();
	verM *= Scale;

	OnGeometryUpdate();
	return *this;
}

ObjectPtr<StaticMesh> StaticMesh::LoadFromObj(const Path& FileName)
{
	if(FileName.extension() == ".obj" || FileName.extension() == ".OBJ")
	{
		MatrixX3d verM;
		MatrixX3i triM;
		if(igl::readOBJ((Path::ContentDir() / FileName).string(), verM, triM))
		{
			return NewObject<StaticMesh>(verM, triM);
		}
		LOG_ERROR("Fail to load mesh from file: {0}", FileName.string());
	}
	LOG_ERROR("Extension of file is not .obj or .OBJ: {0}", FileName.string());
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

void StaticMesh::SetColor(const FColor& Color)
{
	colM.resize(1, 3);
	colM.row(0) = Color;
}

void StaticMesh::CalcNormal()
{
	igl::per_vertex_normals(verM, triM, VertexNormal);
}

bool StaticMesh::CheckNormalValid() const
{
	return VertexNormal.rows() == verM.rows();
}

void StaticMesh::ReverseNormal()
{
	for (int i = 0; i < triM.rows(); i++)
	{
		int tmp_y = triM(i, 1);
		int tmp_z = triM(i, 2);

		triM(i, 1) = tmp_z;
		triM(i, 2) = tmp_y;
	}
}

void StaticMesh::RemoveVertex(int VertexIndex)
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
}

void StaticMesh::RemoveVertices(const TArray<int>& VertexIndices)
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
}

void StaticMesh::RemoveFace(int FaceIndex)
{
	triM.row(FaceIndex) = triM.row(triM.rows() - 1);
	triM.conservativeResize(triM.rows() - 1, 3);
}

void StaticMesh::RemoveFaces(const TArray<int>& FaceIndices)
{
	for (int FaceIndex : FaceIndices)
	{
		triM.row(FaceIndex) = triM.row(triM.rows() - 1);
		triM.conservativeResize(triM.rows() - 1, 3);
	}
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

void StaticMesh::Clear()
{
	verM.resize(0, 3);
	triM.resize(0, 3);
	norM.resize(0, 3);
	colM.resize(0, 3);
	OnGeometryUpdate();
}

bool StaticMesh::IsEmpty() const
{
	return verM.rows() < 3 || triM.rows() < 1;
}