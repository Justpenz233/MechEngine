#pragma once
#include "Algorithm/CurveDistance.h"
#include "Core/CoreMinimal.h"
#include "Components/CurveComponent.h"
#include "Game/Actor.h"

MCLASS(CurveActor)
class ENGINE_API CurveActor : public Actor
{
    REFLECTION_BODY(CurveActor)
protected:
	MPROPERTY()
    ObjectPtr<CurveComponent> CComponent;

public:
	CurveActor() = default;
    CurveActor(const ObjectPtr<Curve>& CurveData);
	CurveActor(const TArray<FVector>& CurveData);
    CurveActor(const TArray<FVector>& CurveData, bool bClosed);
    ObjectPtr<CurveComponent> GetCurveComponent();
	TArray<FVector> SampleWithEqualChordLength(int Samples = 100) const;
    FVector Sample(double u) const;
    FVector SampleIndex(int Index) const;
	int GetPointsNum() const { return CComponent->GetPointsNum(); }

    double CalcSimilarity(const ObjectPtr<CurveActor>& Others) const
	{
		double Result = 0;
		for(double t = 0.; t <= 1.; t += 0.005)
		{
			Result += (Sample(t) - Others->Sample(t)).norm();
		}
		return Result;
    }

	double CalcSimilarity(const ObjectPtr<Curve>& Others) const
	{
		double Result = 0;
		for(double t = 0.; t <= 1.; t += 0.005)
		{
			Result += (Sample(t) - Others->Sample(t)).norm();
		}
		return Result;
	}
};
