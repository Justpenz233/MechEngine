#pragma once
#include "Algorithm/CurveDistance.h"
#include "Core/CoreMinimal.h"
#include "Components/StaticCurveComponent.h"
#include "Game/Actor.h"

MCLASS(CurveActor)
class CurveActor : public Actor
{
    REFLECTION_BODY(CurveActor)
protected:
    ObjectPtr<StaticCurveComponent> CurveComponent;

public:
	CurveActor() = default;
    CurveActor(ObjectPtr<Curve> CurveData);
	CurveActor(TArray<FVector> CurveData);
    CurveActor(TArray<FVector> CurveData, bool bClosed);

    ~CurveActor();

    ObjectPtr<StaticCurveComponent> GetCurveComponent();

    FVector Sample(double u) const;
    FVector SampleIndex(int Index) const;
	int GetPointsNum() const { return CurveComponent->GetPointsNum(); }

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
