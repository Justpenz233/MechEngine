//
// Created by Mavel Li on 12/9/23.
//

#include "StaticCurveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curve/Curve.h"
#include "CoreMinimal.h"
#include "Game/Actor.h"
#include "Mesh/BasicShapesLibrary.h"

StaticCurveComponent::StaticCurveComponent(ObjectPtr<Curve> NewCurve)
{
	CurveData = std::move(NewCurve);
}
void StaticCurveComponent::Init()
{
	StaticMeshComponent::Init();
	if (DrawMode == AsMesh)
	{
		MeshData = GenerateCurveMesh();
		MeshData->CalcNormal();
		Dirty = static_cast<StaticMeshDirtyTag>(Dirty ^ DIRTY_REMESH);
	}
}

void StaticCurveComponent::Remesh()
{
	StaticMeshComponent::Remesh();
	if (DrawMode == AsMesh)
	{
		auto PreMesh = MeshData;
		MeshData = GenerateCurveMesh();
		MeshData->CalcNormal();
		if(PreMesh)
			MeshData->SetMaterial(PreMesh->GetMaterialAsset());
	}
}
void StaticCurveComponent::PostEdit(Reflection::FieldAccessor& Field)
{
	StaticMeshComponent::PostEdit(Field);
	Remesh();
	UploadRenderingData();
}

void StaticCurveComponent::SetDrawMode(CurveDrawMode InDrawMode, int InSampleNum)
{
    DrawMode = InDrawMode;
    if(InSampleNum == -1)
        if(DrawMode == Discrete)
            InSampleNum = CurveData->GetPointsNum();
        else
            InSampleNum = 64;
    SampleNum = InSampleNum;
    MarkAsDirty(DIRTY_REMESH);
}

void StaticCurveComponent::LoadFromFile(String FilePath)
{
    if(CurveData->ReadFromPath(FilePath))
    {
        MarkAsDirty(DIRTY_REMESH);
    }
    else
    {
        LOG_ERROR("CurveComponent read from file {0} failed", FilePath);
    }
}

void StaticCurveComponent::SetRadius(double NewRadius)
{
	Radius = NewRadius;
	Remesh();
}

template<class T>
double StaticCurveComponent::CalcSimilarity(ObjectPtr<T> Others)
{
	double Result = std::numeric_limits<double>::max();
	for (double delta = 0.; delta < 1.; delta += 0.001)
	{
		double Sum = 0;
		for (double u = 0; u <= 1; u += 0.001)
		{
			Sum += (Sample(u) - Others->Sample(u + delta)).norm();
		}
		Result = std::min(Result, Sum);
	}
	return Result;
}


ObjectPtr<StaticMesh> StaticCurveComponent::GenerateCurveMesh()
{
    auto CurveMeshData = BasicShapesLibrary::GenerateCurveMesh(CurveData, Radius, CurveData->IsClosed(), CSG_NUM_N, SampleNum);
    return CurveMeshData;
}