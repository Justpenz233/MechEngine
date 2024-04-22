//
// Created by Siqi Li on 12/9/23.
//
#pragma once
#include "Components/StaticMeshComponent.h"
#include "Core/CoreMinimal.h"
#include "Core/Curve.h"
#include "Mesh/StaticMesh.h"

enum CurveDrawMode
{
    Discrete, Continuous, AsMesh
};

MCLASS(StaticCurveComponent)
class StaticCurveComponent : public StaticMeshComponent
{
    REFLECTION_BODY(StaticCurveComponent)
protected:
    ObjectPtr<Curve> CurveData;
    
    CurveDrawMode DrawMode = AsMesh;

	//----- Mesh mode configuration -----
    MPROPERTY()
    double Radius = 0.05;

	MPROPERTY()
	int SampleNum = 256;

    // Vertex num per points, in a circle
    int CSG_NUM_N = 32;

    ObjectPtr<StaticMesh> GenerateCurveMesh();

public:
    StaticCurveComponent(String FilePath = "");
    StaticCurveComponent(ObjectPtr<Curve>);

	virtual void Init() override;
    virtual void UploadRenderingData() override;
	virtual void Remesh() override;
	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

    void SetDrawMode(CurveDrawMode InDrawMode, int InSampleNum = -1);
    void LoadFromFile(String FilePath);
    void SetRadius(double NewRadius);
    int GetPointsNum() const { return CurveData->GetPointsNum(); }
    int GetEdgeNum() const { return CurveData->GetEdgeNum(); }
    FVector Sample(double u) const { return CurveData->Sample(u); }
    FVector SampleIndex(int Index) const { return CurveData->SampleIndex(Index); }
    FVector& operator [] (int Index) { return CurveData->operator[](Index); }

    template<class T>
    double CalcSimilarity(ObjectPtr<T> Others);
};
