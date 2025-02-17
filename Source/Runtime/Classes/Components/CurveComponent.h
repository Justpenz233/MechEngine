//
// Created by Siqi Li on 12/9/23.
//
#pragma once
#include "Core/CoreMinimal.h"
#include "Curve/Curve.h"
#include "Mesh/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

enum CurveDrawMode
{
    Discrete, AsMesh
};

MCLASS(CurveComponent)
class ENGINE_API CurveComponent : public StaticMeshComponent
{
    REFLECTION_BODY(CurveComponent)
public:
    explicit CurveComponent(ObjectPtr<Curve>);

	virtual void Init() override;
	virtual void Remesh() override;
	virtual void PostEdit(Reflection::FieldAccessor& Field) override;

    void SetDrawMode(CurveDrawMode InDrawMode, int InSampleNum = -1);
    void LoadFromFile(String FilePath);
    void SetRadius(double NewRadius);
    FORCEINLINE int GetPointsNum() const { return CurveData->GetPointsNum(); }
    FORCEINLINE int GetEdgeNum() const { return CurveData->GetEdgeNum(); }
    FORCEINLINE FVector Sample(double u) const { return CurveData->Sample(u); }
    FORCEINLINE FVector SampleIndex(int Index) const { return CurveData->SampleIndex(Index); }
    FVector& operator [] (int Index) const { return CurveData->operator[](Index); }
	FORCEINLINE auto GetCurveData() const { return CurveData; }

    template<class T>
    double CalcSimilarity(ObjectPtr<T> Others);

protected:
    CurveComponent() = default;

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

};
