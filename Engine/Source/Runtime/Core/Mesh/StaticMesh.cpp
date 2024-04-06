//
// Created by Mayn on 8/25/2023.
//

#include "StaticMesh.h"
#include "Math/LinearAlgebra.h"
#include "igl/per_face_normals.h"
#include "igl/per_vertex_normals.h"
#include "igl/per_corner_normals.h"
#include <fstream>
#include <igl/readOBJ.h>

#include "Algorithm/GeometryProcess.h"
#include "igl/copyleft/cgal/is_self_intersecting.h"
#include "Misc/Path.h"

void StaticMesh::UpdateBoundingBox()
{
	BoundingBox = Math::FBox(verM);
}

StaticMesh::StaticMesh(const MatrixX3d& InVerM, const MatrixX3i& InTriM)
{
	verM = InVerM;
	triM = InTriM;
	UpdateBoundingBox();
}

StaticMesh::StaticMesh(MatrixX3d&& InVerM, MatrixX3i&& InTriM)
{
	verM = std::move(InVerM);
	triM = std::move(InTriM);
	UpdateBoundingBox();
}

StaticMesh::StaticMesh(const TArray<Vector3d>& verList, const TArray<Vector3i>& triList)
{
	verM.resize(verList.size(), 3);
	for (int i = 0; i < verList.size(); i++)
		verM.row(i) = verList[i];

	triM.resize(triList.size(), 3);
	for (int i = 0; i < triList.size(); i++)
		triM.row(i) = triList[i];
	UpdateBoundingBox();
}

StaticMesh::StaticMesh(const TArray<double>& verList, const TArray<int>& triList)
{
	verM.resize(verList.size() / 3, 3);
	triM.resize(triList.size() / 3, 3);
	for (int i = 0;i < verList.size();i += 3)
		verM.row(i / 3) = RowVector3d(verList[i], verList[i + 1], verList[i + 2]);
	for (int i = 0; i < triList.size(); i += 3)
		triM.row(i / 3) = Eigen::RowVector3i(triList[i], triList[i + 1], triList[i + 2]);
	UpdateBoundingBox();
}

StaticMesh::StaticMesh(StaticMesh&& Other) noexcept
{
	verM = std::move(Other.verM);
	triM = std::move(Other.triM);
	if (Other.colM.rows() != 0)
		colM = std::move(Other.colM);
	if (Other.norM.rows() != 0)
		norM = std::move(Other.norM);
	UpdateBoundingBox();
}

StaticMesh::StaticMesh(const StaticMesh& Other)
{
	verM = Other.verM;
	triM = Other.triM;
	colM = Other.colM;
	norM = Other.norM;
	UpdateBoundingBox();
}

ObjectPtr<StaticMesh> StaticMesh::operator=(std::shared_ptr<StaticMesh> Other)
{
	verM      = Other->verM;
	triM      = Other->triM;
	if (Other->colM.rows() != 0) colM = Other->colM.eval();
	if (Other->norM.rows() != 0) norM = Other->norM.eval();
	UpdateBoundingBox();
	return Cast<StaticMesh>(GetThis());
}

StaticMesh& StaticMesh::operator=(StaticMesh&& Other) noexcept
{
	verM = std::move(Other.verM);
	triM = std::move(Other.triM);
	if (Other.colM.rows() != 0) colM = std::move(Other.colM);
	if (Other.norM.rows() != 0) norM = std::move(Other.norM);
	UpdateBoundingBox();
	return *this;
}

void StaticMesh::TransformMesh(const Matrix4d& transMat)
{
	if (transMat == Matrix4d::Identity())
		return;

	ParallelFor(verM.rows(),
		[this, &transMat](int i) {
			Vector3d origVer;
			origVer[0] = verM(i, 0);
			origVer[1] = verM(i, 1);
			origVer[2] = verM(i, 2);
			Vector4d NewPos;
			NewPos << origVer, 1.;
			NewPos = transMat * NewPos;
			verM(i, 0) = NewPos[0];
			verM(i, 1) = NewPos[1];
			verM(i, 2) = NewPos[2];
		}, 1e7);
	UpdateBoundingBox();
}
void StaticMesh::SaveOBJ(String FileName)
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
	UpdateBoundingBox();
}

void StaticMesh::Rotate(const FQuat& Rotation)
{
	ParallelFor(verM.rows(), [this, Rotation](int i){
		verM.row(i) = verM.row(i) * Rotation.matrix();
	});
	UpdateBoundingBox();
}

void StaticMesh::RotateEuler(const FVector& RotationEuler)
{
	TransformMesh(MMath::RotationMatrix4FromEulerXYZ(RotationEuler));
	UpdateBoundingBox();
}

void StaticMesh::Scale(const FVector& InScale)
{
	ParallelFor(verM.rows(), [this, InScale](int i) {
		verM(i, 0) *= InScale[0];
		verM(i, 1) *= InScale[1];
		verM(i, 2) *= InScale[2];
	});
	UpdateBoundingBox();
}

void StaticMesh::Scale(double InScale)
{
	Scale({InScale, InScale, InScale});
}

bool StaticMesh::IsSelfIntersect() const
{
	return igl::copyleft::cgal::is_self_intersecting(verM, triM);
}

bool StaticMesh::FillHoles()
{
	return Algorithm::GeometryProcess::FillAllHoles(verM, triM);
}

void StaticMesh::SmoothMesh(int Iteration, bool UseUniform)
{
	Algorithm::GeometryProcess::SmoothMesh(verM, triM, Iteration, UseUniform);
}

void StaticMesh::Normlize()
{
	auto Center = GetBoundingBox().GetCenter();
	auto Scale = 1. / GetBoundingBox().GetSize().maxCoeff();
	verM.rowwise() -= Center.transpose();
	verM *= Scale;

	UpdateBoundingBox();
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

void StaticMesh::SetColor(RowVector3d Color)
{
	colM.resize(1, 3);
	colM.row(0) = Color;
}

void StaticMesh::SetupNormal(const MeshNormalOption& Option)
{
	NormalOption = Option;
	CalcNormal();
}

void StaticMesh::CalcNormal()
{
	if(NormalOption == PerFaceNormal)
		igl::per_face_normals_stable(verM, triM, norM);
	if(NormalOption == PerVertexNormal)
	 	igl::per_vertex_normals(verM, triM, norM);
	if(NormalOption == PerCornerNormal)
		igl::per_corner_normals(verM, triM, 20, norM);
}

bool StaticMesh::CheckNormalValid() const
{
	if (NormalOption == PerFaceNormal)
		return norM.rows() == triM.rows();
	if (NormalOption == PerVertexNormal)
		return norM.rows() == verM.rows();
	if (NormalOption == PerCornerNormal)
		return norM.rows() == triM.rows() * 3;
	if(NormalOption == UserDefinedNormal)
		return norM.rows() != 0;
	return false;
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

void StaticMesh::Clear()
{
	verM.resize(0, 3);
	triM.resize(0, 3);
	norM.resize(0, 3);
	colM.resize(0, 3);
	UpdateBoundingBox();
}

bool StaticMesh::IsEmpty() const
{
	return verM.rows() < 3 || triM.rows() < 1;
}

void StaticMesh::ClearDegenerate()
{
	return;
	auto CalcTriangleArea = [&](Vector3i Triangle)
	{
		Vector3d AB = verM.row(Triangle.x()) - verM.row(Triangle.y());
		Vector3d AC = verM.row(Triangle.z()) - verM.row(Triangle.x());
		return 0.5 * abs(AB.cross(AC).stableNorm());
	};

	int LastFaceIndex = GetFaceNum() - 1;
	for(int i = 0;i <= LastFaceIndex;i ++)
	{
		if(CalcTriangleArea(triM.row(i)) < 1e-10)
		{
			triM.row(i).swap(triM.row(LastFaceIndex --));
		}
	}
	LOG_INFO("Clear degenerate Triangles.  Clear count: {0}", triM.rows() - LastFaceIndex - 1);
	triM.conservativeResize(LastFaceIndex + 1, 3);
	UpdateBoundingBox();
}


