#include "BasicShapesLibrary.h"
///////////////////////////////////////////////////////////////
//
// BasicShapesLibrary.cpp
//
//   Construct libigl mesh of various primitive shapes
//
// by Yingjie Cheng and Peng Song
//
// 28/Nov/2020
//
// Modified by CHENG 2022/Aug/08
//
///////////////////////////////////////////////////////////////

#include "CoreMinimal.h"
#include "Math/FTransform.h"
#include "Curve/Curve.h"
#include "Object/Object.h"
#include "StaticMesh.h"
#include "tinysplinecxx.h"
///=========================================================================================///
///                                        Cuboid
///=========================================================================================///


ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCuboid(Vector3d size)
{
	TArray<Vector3d> verList;
	TArray<Vector3i> triList;

	/////////////////////////////////////////////////////////////////////////
	/// 1. Computer vertices of the cuboid

	double minX = -0.5f * size[0];
	double minY = -0.5f * size[1];
	double minZ = -0.5f * size[2];

	double maxX = 0.5f * size[0];
	double maxY = 0.5f * size[1];
	double maxZ = 0.5f * size[2];

	verList.emplace_back(minX, minY, maxZ);
	verList.emplace_back(maxX, minY, maxZ);
	verList.emplace_back(maxX, maxY, maxZ);
	verList.emplace_back(minX, maxY, maxZ);

	verList.emplace_back(minX, minY, minZ);
	verList.emplace_back(maxX, minY, minZ);
	verList.emplace_back(maxX, maxY, minZ);
	verList.emplace_back(minX, maxY, minZ);

	/////////////////////////////////////////////////////////////////////////
	/// 2. Computer triangles of the cuboid

	triList.emplace_back(5, 4, 7);
	triList.emplace_back(5, 7, 6);
	triList.emplace_back(7, 2, 6);
	triList.emplace_back(7, 3, 2);

	triList.emplace_back(1, 0, 4);
	triList.emplace_back(1, 3, 0);
	triList.emplace_back(5, 6, 2);
	triList.emplace_back(5, 2, 1);

	triList.emplace_back(4, 5, 1);
	triList.emplace_back(1, 2, 3);
	triList.emplace_back(0, 3, 4);
	triList.emplace_back(4, 3, 7);

	/////////////////////////////////////////////////////////////////////////
	/// 3. Construct a triangular mesh of the sphere
	return NewObject<StaticMesh>(verList, triList);
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCuboid(Vector3d A, Vector3d B, Vector2d Size)
{
	double Length = (B - A).norm();
	auto   Mesh = GenerateCuboid({Length, Size[0], Size[1]});

	// Compute the translation matrix
	Vector3d   center = 0.5f * (A + B);
	Vector3d   TArray = (B - A).normalized();
	FQuat	   Rotation = FQuat::FromTwoVectors(Vector3d{ 1., 0., 0. }, TArray);
	FTransform Transform = FTransform::Identity();
	Transform.SetRotation(Rotation);
	Transform.SetTranslation(center);

	// Transform the cylinder to its target position and orientation
	Mesh->TransformMesh(Transform.GetMatrix());
	return Mesh;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCuboid(double size)
{
	return GenerateCuboid({size, size, size});
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GeneratePlane(const FVector2& Size)
{
	TArray<Vector3d> verList;
	TArray<Vector3i> triList;

	/////////////////////////////////////////////////////////////////////////
	/// 1. Computer vertices of the cuboid

	double minX = -0.5 * Size[0];
	double minY = -0.5 * Size[1];
	double maxX = 0.5 * Size[0];
	double maxY = 0.5 * Size[1];

	verList.emplace_back(minX, minY, 0);
	verList.emplace_back(maxX, minY, 0);
	verList.emplace_back(maxX, maxY, 0);
	verList.emplace_back(minX, maxY, 0);

	triList.emplace_back(0, 1, 2);
	triList.emplace_back(0, 2, 3);

	return NewObject<StaticMesh>(verList, triList);
}
ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateFan(double Radius, double Height, double StartAngle, double EndAngle, int Sample)
{
	TArray<Vector3d> verList;
	TArray<Vector3i> triList;


	verList.emplace_back(0, 0, Height/2);
	for(int i=0;i<=Sample;i++)
	{
		double theta = (EndAngle - StartAngle)/Sample * 1.0 * i + StartAngle;
		verList.emplace_back(sin(theta)*Radius,  cos(theta)*Radius, Height/2);
	}
	verList.emplace_back(0, 0, -Height/2);
	for(int i=0;i<=Sample;i++)
	{
		double theta = (EndAngle - StartAngle)/Sample * 1.0 * i + StartAngle;
		verList.emplace_back(sin(theta)*Radius, cos(theta)*Radius, -Height/2);
	}

	/////////////////////////////////////////////////////////////////////////
	/// 2. Computer triangles of the fan
	for(int i=0;i<Sample;i++)
		triList.emplace_back(0,  i+2, i+1);
	for(int i=Sample+2; i<Sample*2+2;i++)
		triList.emplace_back(Sample+2, i+1, i+2);
	for(int i=0;i<=Sample;i++)
	{
		triList.emplace_back(i, i+Sample+3, i+Sample+2);
		triList.emplace_back(i, i+1, i+Sample+3);
	}
	triList.emplace_back(Sample+1, Sample+2, Sample+Sample+3);
	triList.emplace_back(Sample+1, 0, Sample+2);

	/////////////////////////////////////////////////////////////////////////
	/// 3. Construct a triangular mesh of the cylinder
	return NewObject<StaticMesh>(verList, triList);
}

///=========================================================================================///
///                                         Sphere
///=========================================================================================///


ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCurveMesh(const TArray<FVector>& CurveData, double Radius, bool bClosed, bool bUseCubiod, int RingSample,
	int SampleNum)
{
    ObjectPtr<Curve> CurveT = NewObject<Curve>(CurveData, bClosed);
	if(bUseCubiod)
		return GenerateCurveMeshCubiod(CurveT, Radius, bClosed, SampleNum);
	else
		return GenerateCurveMesh(CurveT, Radius, bClosed, RingSample, SampleNum);
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCurveMesh(ObjectPtr<Curve> CurveData, double Radius, bool bClosed, int RingSample,
	int SampleNum)
{
	bool IsClosedCurve = bClosed;
	auto LinesData = CurveData->GetCurveData();
	TArray<double> Data(LinesData.size() * 3);
	std::memcpy(Data.data(), LinesData.data(), LinesData.size() * sizeof(double) * 3);
	auto Spline = tinyspline::BSpline::interpolateCatmullRom(Data, 3);

	auto knots = Spline.equidistantKnotSeq(SampleNum);
	auto Frames = Spline.computeRMF(knots);


	TArray<Vector3d> G_verList;
	for (int i = 0; i < SampleNum; i++)
	{
		auto P = Frames.at(i).position();
		auto N = Frames.at(i).normal();
		auto BN = Frames.at(i).binormal();
		for (int j = 0; j < RingSample; j++)
		{
			Vector3d TubeP;
			double	 theta = j * 2.0 * M_PI / RingSample;
			FVector	 Pos = FVector{ P.x(), P.y(), P.z() };
			FVector  Normal = FVector{ N.x(), N.y(), N.z() };
			FVector  Binormal = FVector{ BN.x(), BN.y(), BN.z() };
			FVector Vertex = Radius * cos(theta) * Normal + Radius * sin(theta) * Binormal;
			TubeP = Pos + Radius * Vertex.normalized();
			G_verList.push_back(TubeP);
		}
	}

	/// triList
	TArray<Vector3i> G_triList;
	int				 StartId = 0;
	for (int itr = 0; itr < SampleNum - 1; itr++)
	{
		for (int j = 0; j < RingSample - 1; j++)
		{
			G_triList.emplace_back(StartId + j, StartId + RingSample + j, StartId + RingSample + 1 + j);
			G_triList.emplace_back(StartId + j, StartId + RingSample + 1 + j, StartId + 1 + j);
		}
		G_triList.emplace_back(StartId + RingSample - 1, StartId + 2 * RingSample - 1, StartId + RingSample);
		G_triList.emplace_back(StartId + RingSample - 1, StartId + RingSample, StartId);
		StartId += RingSample;
	}

	if (IsClosedCurve)
	{
		for (int j = 0; j < RingSample - 1; j++)
		{
			G_triList.emplace_back(StartId + j, j, j + 1);
			G_triList.emplace_back(StartId + j, j + 1, StartId + j + 1);
		}
		G_triList.emplace_back(StartId + RingSample - 1, RingSample - 1, 0);
		G_triList.emplace_back(StartId + RingSample - 1, 0, StartId);
	}
	else
	{
		G_verList.push_back(CurveData->SampleIndex(0));
		int CenterIndex = G_verList.size() - 1;
		for (int i = 0; i < RingSample; i++)
			G_triList.emplace_back(i, (i + 1) % RingSample, CenterIndex);

		G_verList.push_back(CurveData->SampleIndex(CurveData->GetPointsNum() - 1));
		CenterIndex = G_verList.size() - 1;
		for (int i = 0; i < RingSample; i++)
		{
			int IndexBase = RingSample * (SampleNum - 1);
			G_triList.emplace_back(IndexBase + i, CenterIndex, IndexBase + ((i + 1) % RingSample));
		}
	}

	auto CurveMeshData = NewObject<StaticMesh>(G_verList, G_triList);
	CurveMeshData->ReverseNormal();
	return CurveMeshData;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCurveMeshCubiod(ObjectPtr<Curve> CurveData, double Radius, bool bClosed, int SampleNum)
{
	bool IsClosedCurve = bClosed;

	auto		   LinesData = CurveData->GetCurveData();
	TArray<double> Data(LinesData.size() * 3);
	std::memcpy(Data.data(), LinesData.data(), LinesData.size() * sizeof(double) * 3);
	auto Spline = tinyspline::BSpline::interpolateCatmullRom(Data, 3);

	auto knots = Spline.equidistantKnotSeq(SampleNum);
	auto Frames = Spline.computeRMF(knots);

	TArray<Vector3d> G_verList;
	for (int i = 0; i < SampleNum; i++)
	{
		double u = i * 1.0 / (SampleNum - 1);

		Vector3d N_1;
		N_1 = CurveData->SampleTangent(u);
		FVector Up = FVector{ 0, 0, 1 };
		FVector Right = -N_1.cross(Up).normalized();
		FVector Pos = CurveData->Sample(u);
		G_verList.emplace_back(Pos + Radius * Up + Radius * Right);
		G_verList.emplace_back(Pos + Radius * Up - Radius * Right);
		G_verList.emplace_back(Pos - Radius * Up - Radius * Right);
		G_verList.emplace_back(Pos - Radius * Up + Radius * Right);
	}

	/// triList
	TArray<Vector3i> G_triList;
	int				 StartId = 0;
	for (int itr = 0; itr < SampleNum - 1; itr++)
	{
		for (int j = 0; j < 4; j++)
		{
			int Next = (j + 1) % 4;
			G_triList.emplace_back(StartId + j, StartId + 4 + j, StartId + 4 + Next);
			G_triList.emplace_back(StartId + j, StartId + 4 + Next, StartId + Next);
		}
		StartId += 4;
	}

	if (IsClosedCurve)
	{
		for (int j = 0; j < 4; j++)
		{
			int Next = (j + 1) % 4;
			int Bottom = G_verList.size() - 4;
			G_triList.emplace_back(Bottom + j, Next, Bottom + Next);
			G_triList.emplace_back(Bottom + j, j, Next);
		}
	}
	else
	{
		G_triList.emplace_back(0, 1, 2);
		G_triList.emplace_back(0, 2, 3);

		int i = G_verList.size() - 4;
		G_triList.emplace_back(i + 1, i + 3, i + 2);
		G_triList.emplace_back(i + 1, i, i + 3);
	}

	auto CurveMeshData = NewObject<StaticMesh>(G_verList, G_triList);
	CurveMeshData->ReverseNormal();
	return CurveMeshData;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateExtrudeMesh(const TArray<FVector>& CurveData, const TFunction<FVector2(double)>& ContourSample, int ControuSample, int CurveSample)
{
	TArray<double> Data(CurveData.size() * 3);
	std::memcpy(Data.data(), CurveData.data(), CurveData.size() * sizeof(double) * 3);
	auto Spline = tinyspline::BSpline::interpolateCatmullRom(Data, 3);

	auto knots = Spline.equidistantKnotSeq(CurveSample);
	auto Frames = Spline.computeRMF(knots);


	TArray<Vector3d> G_verList;
	for (int i = 0; i < CurveSample; i++)
	{
		auto P = Frames.at(i).position();
		auto N = Frames.at(i).normal();
		auto BN = Frames.at(i).binormal();
		for (int j = 0; j < ControuSample; j++)
		{
			FVector	 Pos = FVector{ P.x(), P.y(), P.z() };
			FVector  Normal = FVector{ N.x(), N.y(), N.z() };
			FVector  Binormal = FVector{ BN.x(), BN.y(), BN.z() };
			double t = j * 1.0 / ControuSample;
			FVector2 O = ContourSample(t);
			G_verList.push_back(Pos + O.y() * Normal + O.x() * Binormal);
		}
	}

	/// triList
	TArray<Vector3i> G_triList;
	int				 StartId = 0;
	for (int itr = 0; itr < CurveSample - 1; itr++)
	{
		for (int j = 0; j < ControuSample - 1; j++)
		{
			G_triList.emplace_back(StartId + j, StartId + ControuSample + j, StartId + ControuSample + 1 + j);
			G_triList.emplace_back(StartId + j, StartId + ControuSample + 1 + j, StartId + 1 + j);
		}
		G_triList.emplace_back(StartId + ControuSample - 1, StartId + 2 * ControuSample - 1, StartId + ControuSample);
		G_triList.emplace_back(StartId + ControuSample - 1, StartId + ControuSample, StartId);
		StartId += ControuSample;
	}


	for (int j = 0; j < ControuSample - 1; j++)
	{
		G_triList.emplace_back(StartId + j, j, j + 1);
		G_triList.emplace_back(StartId + j, j + 1, StartId + j + 1);
	}
	G_triList.emplace_back(StartId + ControuSample - 1, ControuSample - 1, 0);
	G_triList.emplace_back(StartId + ControuSample - 1, 0, StartId);



	auto CurveMeshData = NewObject<StaticMesh>(G_verList, G_triList);
	CurveMeshData->ReverseNormal();
	return CurveMeshData;

}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateSphere(double Radius, int Sample)
{
	TArray<Vector3d> verList;
	TArray<Vector3i> triList;

	/////////////////////////////////////////////////////////////////////////
	/// 1. Computer vertices of the sphere

	verList.emplace_back(0, 0, Radius);
	for (int i = 1; i < Sample; i++) {
		double   alpha = M_PI / 2.0 - i * M_PI / Sample;
		for (int j     = 0; j < Sample; j++) {
			double beta = j * 2.0 * M_PI / Sample;
			double x, y, z;
			x = Radius * cos(alpha) * cos(beta);
			y = Radius * cos(alpha) * sin(beta);
			z = Radius * sin(alpha);
			verList.emplace_back(x, y, z);
		}
	}
	verList.emplace_back(0, 0, -Radius);


	/////////////////////////////////////////////////////////////////////////
	/// 2. Computer triangles of the sphere

	// Top ring of triangles
	for (int j = 0; j < Sample - 1; j++) {
		triList.emplace_back(0, j + 1, j + 2);
	}
	triList.emplace_back(0, Sample, 1);

	// Middle rings of triangles
	int      StartId = 1;
	for (int i       = 1; i < Sample - 1; i++) {
		for (int j = 0; j < Sample - 1; j++) {
			triList.emplace_back(StartId + j, StartId + Sample + j, StartId + Sample + j + 1);
			triList.emplace_back(StartId + j, StartId + Sample + j + 1, StartId + 1 + j);
		}
		triList.emplace_back(StartId + Sample - 1, StartId + 2 * Sample - 1, StartId + Sample);
		triList.emplace_back(StartId + Sample - 1, StartId + Sample, StartId);
		StartId += Sample;
	}

	// Bottom ring of triangles
	int      verNum = verList.size();
	for (int j      = 0; j < Sample - 1; j++) {
		triList.emplace_back(verNum - 1, j + 1 + StartId, j + StartId);
	}
	triList.emplace_back(verNum - 1, StartId, StartId + Sample - 1);

	return NewObject<StaticMesh>(verList, triList);
}
ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateHemisphere(double Radius, bool bTopHalf, int Sample)
{
	TArray<Vector3d> Vertices;
	TArray<Vector3i> Indices;

	Vertices.emplace_back(0, 0, bTopHalf ? Radius : -Radius);
	for (int i = 0; i < Sample; i++)
	{
        double  theta = M_PI_2 - (i + 1) * M_PI_2 / Sample;
		for (int j = 0; j < Sample; j++)
		{
			double phi = j * 2 * M_PI / Sample;
			double x = Radius * cos(theta) * cos(phi);
			double y = Radius * cos(theta) * sin(phi);
			double z = Radius * sin(theta) * (bTopHalf ? 1 : -1);
			Vertices.emplace_back(x, y, z);
		}
	}
	Vertices.emplace_back(0, 0, 0);

	for (int i = 0; i < Sample - 1; i ++)
		Indices.emplace_back(0, i + 1, i + 2);
	Indices.emplace_back(0, Sample, 1);

	int StartId = 1;
	for (int i = 1; i < Sample; i ++) {
		for (int j = 0; j < Sample - 1; j++) {
			Indices.emplace_back(StartId + j, StartId + Sample + j, StartId + Sample + j + 1);
			Indices.emplace_back(StartId + j, StartId + Sample + j + 1, StartId + 1 + j);
		}
		Indices.emplace_back(StartId + Sample - 1, StartId + 2 * Sample - 1, StartId + Sample);
		Indices.emplace_back(StartId + Sample - 1, StartId + Sample, StartId);
		StartId += Sample;
	}

	for (int j = 0; j < Sample - 1; j++) {
		Indices.emplace_back(Vertices.size() - 1, j + 1 + StartId, j + StartId);
	}
	Indices.emplace_back(Vertices.size() - 1, StartId, StartId + Sample - 1);

	auto Result = NewObject<StaticMesh>(Vertices, Indices);
	if(!bTopHalf)
		Result->ReverseNormal();
	return Result;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCylinder(double length, double radius, int Samples)
{
	TArray<Vector3d> verList;
	TArray<Vector3i> triList;

	/////////////////////////////////////////////////////////////////////////
	/// 1. Computer vertices of the cylinder

	double halfLen = 0.5f * length;

	// Center point of the cap
	verList.emplace_back(0, 0, -halfLen);
	verList.emplace_back(0, 0, halfLen);


	// Smaple points on the body
	for (int i = 0; i < Samples; i++)
	{
		for(int j = 0;j < Samples; j ++)
		{
			double beta = j * 2.0 * M_PI / Samples;

			double x, y, z;
			x = radius * cos(beta);
			y = radius * sin(beta);
			z = i * length / (Samples - 1) - halfLen;

			verList.emplace_back(x, y, z);
		}
	}

	for (int h = 0; h < Samples - 1; ++ h) {
		for (int i = 0; i < Samples; ++ i) {
			int start = h * Samples + 2;
			int cur = i + start;
			int next = (i + 1) % Samples + start;
			triList.emplace_back(cur, next, cur + Samples);
			triList.emplace_back(next, next + Samples, cur + Samples);
		}
	}

	// Generate indices for the bottom cap
	for (int i = 0; i < Samples; i ++) {
		int next = (i + 1) % Samples + 2;
		int cur = i + 2;
		triList.emplace_back(0, next, cur);
	}

	// Generate indices for the top cap
	for (int i = 0; i < Samples; ++i) {
		int start = (Samples - 1) * Samples + 2;
		int next = (i + 1) % Samples + start;
		int cur = i + start;
		triList.emplace_back(1, cur, next);
	}

    return NewObject<StaticMesh>(verList, triList);
}

auto BasicShapesLibrary::GenerateCylinder(const Vector3d& A, Vector3d B, double Radius, int Samples) -> ObjectPtr<StaticMesh>
{
	// Create a cylinder with the given radius and computed length
	double Length = (B - A).norm();
	auto   Mesh = GenerateCylinder(Length, Radius, Samples);

	// Compute the translation matrix
	Vector3d   center = 0.5f * (A + B);
	Vector3d   TArray = (B - A).normalized();
	FQuat	   Rotation = FQuat::FromTwoVectors(Vector3d{ 0., 0., 1. }, TArray);
	FTransform Transform = FTransform::Identity();
	Transform.SetRotation(Rotation);
	Transform.SetTranslation(center);

	// Transform the cylinder to its target position and orientation
	Mesh->TransformMesh(Transform.GetMatrix());
	return Mesh;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCapsule(double Radius, double Height, int Sample, int RingSample)
{
	TArray<FVector>	 Vertices;
	TArray<Vector3i> Indices;
	double			 HalfHeight = Height * 0.5;
	// Top Hemisphere
	{
		Vertices.emplace_back(0, 0, HalfHeight + Radius);
		for (int i = 0; i < RingSample; i++)
		{
			double  theta = M_PI_2 - (i + 1) * M_PI_2 / RingSample;
			for (int j = 0; j < RingSample; j++)
			{
				double phi = j * 2 * M_PI / RingSample;
				double x = Radius * cos(theta) * cos(phi);
				double y = Radius * cos(theta) * sin(phi);
				double z = Radius * sin(theta) + Height / 2;
				Vertices.emplace_back(x, y, z);
			}
		}

		for (int i = 0; i < RingSample - 1; i++)
			Indices.emplace_back(0, i + 1, i + 2);
		Indices.emplace_back(0, RingSample, 1);

		int StartId = 1;
		for (int i = 1; i < RingSample; i++)
		{
			for (int j = 0; j < RingSample - 1; j++)
			{
				Indices.emplace_back(StartId + j, StartId + RingSample + j, StartId + RingSample + j + 1);
				Indices.emplace_back(StartId + j, StartId + RingSample + j + 1, StartId + 1 + j);
			}
			Indices.emplace_back(StartId + RingSample - 1, StartId + 2 * RingSample - 1, StartId + RingSample);
			Indices.emplace_back(StartId + RingSample - 1, StartId + RingSample, StartId);
			StartId += RingSample;
		}
	}
	int TopHatVertexNum = Vertices.size();

	// Body
	{
		int IndexBias = Vertices.size();
		for (int i = 0; i < Sample; i++)
		{
			for(int j = 0;j < RingSample; j ++)
			{
				double beta = j * 2.0 * M_PI / RingSample;

				double x, y, z;
				x = Radius * cos(beta);
				y = Radius * sin(beta);
				z = HalfHeight - (i + 1) * Height / (Sample + 1);
				Vertices.emplace_back(x, y, z);
			}
		}
		for (int h = 0; h < Sample - 1; ++ h) {
			for (int i = 0; i < RingSample; ++ i) {
				int start = h * RingSample + IndexBias;
				int cur = i + start;
				int next = (i + 1) % RingSample + start;
				Indices.emplace_back(cur, cur + RingSample, next + RingSample);
				Indices.emplace_back(cur, next + RingSample, next);
			}
		}
	}
	int BodyVertexStart = Vertices.size();

	// Bottom Hemisphere
	{
		int IndexBias = Vertices.size();
		Vertices.emplace_back(0, 0, - HalfHeight - Radius);
		for (int i = 0; i < RingSample; i++)
		{
			double  theta = M_PI_2 - (i + 1) * M_PI_2 / RingSample;
			for (int j = 0; j < RingSample; j++)
			{
				double phi = j * 2 * M_PI / RingSample;
				double x = Radius * cos(theta) * cos(phi);
				double y = Radius * cos(theta) * sin(phi);
				double z = -(Radius * sin(theta) + Height / 2);
				Vertices.emplace_back(x, y, z);
			}
		}
		for (int i = 0; i < RingSample - 1; i++)
			Indices.emplace_back(0 + IndexBias, i + 2 + IndexBias, i + 1 + IndexBias);
		Indices.emplace_back(0 + IndexBias, 1 + IndexBias, RingSample + IndexBias);

		int StartId = 1 + IndexBias;
		for (int i = 1; i < RingSample; i++)
		{
			for (int j = 0; j < RingSample - 1; j++)
			{
				Indices.emplace_back(StartId + 1 + j, StartId + RingSample + j + 1, StartId + RingSample + j);
				Indices.emplace_back(StartId + j, StartId + 1 + j, StartId + RingSample + j);
			}
			Indices.emplace_back(StartId + RingSample - 1, StartId + RingSample, StartId + 2 * RingSample - 1);
			Indices.emplace_back(StartId + RingSample - 1, StartId, StartId + RingSample);
			StartId += RingSample;
		}
	}

	// Seal the top and body
	for (int i = 0; i < RingSample; i++)
	{
		int start = i + TopHatVertexNum - RingSample;
		int next = (i + 1) % RingSample + TopHatVertexNum - RingSample;
		int bottom = start + RingSample;
		int bottomNext = next + RingSample;
		Indices.emplace_back(start, bottom, bottomNext);
		Indices.emplace_back(start, bottomNext, next);
	}

	// Seal bottom with body
	for (int i = 0; i < RingSample; i++)
	{
		int start = i + BodyVertexStart - RingSample;
		int next = (i + 1) % RingSample + BodyVertexStart - RingSample;
		int bottom = i + Vertices.size() - RingSample;
		int bottomNext = (i + 1) % RingSample + Vertices.size() - RingSample;
		Indices.emplace_back(start, bottom, bottomNext);
		Indices.emplace_back(start, bottomNext, next);
	}

	return NewObject<StaticMesh>(Vertices, Indices);
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateOneHatCapsule(double Radius, double Height, bool bTopHat, int Sample, int RingSample)
{
	TArray<FVector>	 Vertices;
	TArray<Vector3i> Indices;
	double			 HalfHeight = Height * 0.5;
	// Top Hemisphere
	{
		Vertices.emplace_back(0, 0, HalfHeight + Radius);
		for (int i = 0; i < RingSample; i++)
		{
			double theta = M_PI_2 - (i + 1) * M_PI_2 / RingSample;
			for (int j = 0; j < RingSample; j++)
			{
				double phi = j * 2 * M_PI / RingSample;
				double x = Radius * cos(theta) * cos(phi);
				double y = Radius * cos(theta) * sin(phi);
				double z = Radius * sin(theta) + Height / 2;
				Vertices.emplace_back(x, y, z);
			}
		}

		for (int i = 0; i < RingSample - 1; i++)
			Indices.emplace_back(0, i + 1, i + 2);
		Indices.emplace_back(0, RingSample, 1);

		int StartId = 1;
		for (int i = 1; i < RingSample; i++)
		{
			for (int j = 0; j < RingSample - 1; j++)
			{
				Indices.emplace_back(StartId + j, StartId + RingSample + j, StartId + RingSample + j + 1);
				Indices.emplace_back(StartId + j, StartId + RingSample + j + 1, StartId + 1 + j);
			}
			Indices.emplace_back(StartId + RingSample - 1, StartId + 2 * RingSample - 1, StartId + RingSample);
			Indices.emplace_back(StartId + RingSample - 1, StartId + RingSample, StartId);
			StartId += RingSample;
		}
	}
	int TopHatVertexNum = Vertices.size();

	// Body
	{
		int IndexBias = Vertices.size();
		for (int i = 0; i < Sample; i++)
		{
			for (int j = 0; j < RingSample; j++)
			{
				double beta = j * 2.0 * M_PI / RingSample;

				double x, y, z;
				x = Radius * cos(beta);
				y = Radius * sin(beta);
				z = HalfHeight - (i + 1) * Height / Sample;
				Vertices.emplace_back(x, y, z);
			}
		}
		for (int h = 0; h < Sample - 1; ++h)
		{
			for (int i = 0; i < RingSample; ++i)
			{
				int start = h * RingSample + IndexBias;
				int cur = i + start;
				int next = (i + 1) % RingSample + start;
				Indices.emplace_back(cur, cur + RingSample, next + RingSample);
				Indices.emplace_back(cur, next + RingSample, next);
			}
		}
		Vertices.emplace_back(0, 0, -HalfHeight);
		int CenterIndex = Vertices.size() - 1;
		for (int i = 0; i < RingSample; i++)
		{
			int next = (i + 1) % RingSample + CenterIndex - RingSample;
			int cur = i + CenterIndex - RingSample;
			Indices.emplace_back(CenterIndex, next, cur);
		}
	}

	// Seal the top and body
	for (int i = 0; i < RingSample; i++)
	{
		int start = i + TopHatVertexNum - RingSample;
		int next = (i + 1) % RingSample + TopHatVertexNum - RingSample;
		int bottom = start + RingSample;
		int bottomNext = next + RingSample;
		Indices.emplace_back(start, bottom, bottomNext);
		Indices.emplace_back(start, bottomNext, next);
	}

	auto Result = NewObject<StaticMesh>(Vertices, Indices);
	if (!bTopHat)
		Result->RotateEuler({ M_PI, 0, 0 });
	return Result;
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateHollowCylinder(const Vector3d& A, const Vector3d& B, double OuterRadius, double InnerRadius, int Samples)
{
	double Length = (B - A).norm();
	auto   Mesh = GenerateHollowCylinder(OuterRadius, InnerRadius, Length, Samples);

	// Compute the translation matrix
	Vector3d   center = 0.5f * (A + B);
	Vector3d   TArray = (B - A).normalized();
	FQuat	   Rotation = FQuat::FromTwoVectors(Vector3d{ 0., 0., 1. }, TArray);
	FTransform Transform = FTransform::Identity();
	Transform.SetRotation(Rotation);
	Transform.SetTranslation(center);

	// Transform the cylinder to its target position and orientation
	Mesh->TransformMesh(Transform.GetMatrix());
	return Mesh;

}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateHollowCylinder(double OuterRadius, double InnerRadius, double Height, int Sample)
{
    TArray<FVector> vertices;
    TArray<Vector3i> indices;

    double angleStep = (2. * M_PI) / Sample;
    double halfHeight = Height / 2.;

    // Create a hollow cylinder with outerRadius and InnerRadius and height
    for (int i = 0; i < Sample; ++i) {
        float angle = i * angleStep;
        float x = cos(angle);
        float y = sin(angle);

        // Outer top vertex
        vertices.emplace_back(OuterRadius * x, OuterRadius * y, halfHeight);
        // Outer bottom vertex
        vertices.emplace_back(OuterRadius * x, OuterRadius * y, -halfHeight);
        // Inner top vertex
        vertices.emplace_back(InnerRadius * x, InnerRadius * y, halfHeight);
        // Inner bottom vertex
        vertices.emplace_back(InnerRadius * x, InnerRadius * y, -halfHeight);
    }
    // Create indices
    for (int i = 0; i < Sample; ++i) {
        int next = (i + 1) % Sample;

        // Outer surface
        indices.emplace_back(i * 4, i * 4 + 1, next * 4);
        indices.emplace_back(next * 4, i * 4 + 1, next * 4 + 1);

        // Inner surface
        indices.emplace_back(i * 4 + 2, next * 4 + 2, i * 4 + 3);
        indices.emplace_back(next * 4 + 2, next * 4 + 3, i * 4 + 3);

        // Top ring
        indices.emplace_back(i * 4, next * 4, i * 4 + 2);
        indices.emplace_back(next * 4, next * 4 + 2, i * 4 + 2);

        // Bottom ring
        indices.emplace_back(i * 4 + 1, i * 4 + 3, next * 4 + 1);
        indices.emplace_back(next * 4 + 1, i * 4 + 3, next * 4 + 3);
    }

    return NewObject<StaticMesh>(vertices, indices);
}

ObjectPtr<StaticMesh>
BasicShapesLibrary::TriangularSurface(int NumU, int NumV, std::function<FVector(double, double)> SampleFunc,
                                      bool ReverseNormal, bool ClosedSurface)
{
    assert(NumU >= 3 && NumV >= 2);

    bool IsClosedPolygon    = ((SampleFunc(1., 0.) - SampleFunc(0., 0.)).norm() < 0.00001) | ClosedSurface;
    // When closed, NumU gap with NumU lines, when open minus 1
    int  InnerTriangleNum   = IsClosedPolygon? NumU * (NumV - 1) * 2 : (NumU - 1) * (NumV - 1) * 2;
    int  VertexNum          = NumU * NumV;
    MatrixX3d VerM;
    MatrixX3i TriM;
    VerM.resize(VertexNum, 3);
    TriM.resize(InnerTriangleNum, 3);

    int    VertexIndex = 0;
    double StepU        = IsClosedPolygon ? 1. / (double)NumU : 1. / (double) (NumU - 1);
    double StepV        = 1. / (double)(NumV - 1);


    for (int VIndex = 0; VIndex < NumV; VIndex++) {
        for (int UIndex = 0; UIndex < NumU; UIndex++) {
            double u = (double)UIndex * StepU;
            double v = (double)VIndex * StepV;
            if(VIndex == NumV - 1) v = 1.;
            VerM.row(VertexIndex++) = SampleFunc(u, v);
        }
    }
    int TriangleIndex = 0;

    for (int VIndex = 0; VIndex < NumV - 1; VIndex++) {
        for (int UIndex = 0; UIndex < NumU; UIndex++) {
            int This    = UIndex + VIndex * NumU;
            int Right   = (UIndex == NumU - 1)? This - NumU + 1 : This + 1;
            int Top     = This + NumU;
            int TopLeft = (UIndex == 0)? This + NumU + NumU - 1 : This + NumU - 1;

            assert(This < VertexNum && Right < VertexNum && Top < VertexNum && TopLeft < VertexNum);
            assert(This >= 0 && Right >= 0 && Top >= 0 && TopLeft >= 0);

            if (IsClosedPolygon || UIndex != 0)
                if(ReverseNormal)
                    TriM.row(TriangleIndex++) = Vector3i{This, TopLeft,Top};
                else
                    TriM.row(TriangleIndex++) = Vector3i{Top, TopLeft,This};
            if (IsClosedPolygon || UIndex != NumU - 1)
                if(ReverseNormal)
                    TriM.row(TriangleIndex++) = Vector3i{Top, Right, This};
                else
                    TriM.row(TriangleIndex++) = Vector3i{This, Right, Top};

        }
    }

    assert(InnerTriangleNum == TriangleIndex);
    return NewObject<StaticMesh>(std::move(VerM), std::move(TriM));
}

ObjectPtr<StaticMesh> BasicShapesLibrary::GenerateCone(double Radius, double Height, bool SealBottom, int SampleRing, int SampleHeight)
{
    auto SampleFunction = [&](double u, double v)
    {
        if(u < 0) u += 1.;
        if(u > 1.) u -= 1.;
        v = 1. - v;
        return FVector{Radius * cos(u * M_PI * 2.0) * v, Radius * sin(u * M_PI * 2.0) * v, (1. - v) * Height};
    };

    auto Mesh = TriangularSurface(SampleRing, SampleHeight, SampleFunction, false, true);
    if(SealBottom)
    {
        int VertexNum = Mesh->GetVertexNum();
        auto VerM = Mesh->GetVertices();
        auto TriM = Mesh->GetTriangles();
        VerM.conservativeResize(VertexNum + 1, 3);
        VerM.row(VertexNum) = FVector{0, 0, 0};
        TriM.conservativeResize(TriM.rows() + SampleRing, 3);
        for(int i = 0; i < SampleRing; i ++)
        {
            int next = (i + 1) % SampleRing;
            TriM.row(TriM.rows() - SampleRing + i) = Vector3i{i, VertexNum, next};
        }
        Mesh->SetGeometry(VerM, TriM);
    }
    return Mesh;
}