#include "CurveActor.h"
#include "Components/CurveComponent.h"
#include "Object/Object.h"


CurveActor::CurveActor(const ObjectPtr<Curve>& CurveData)
{
    CComponent = AddComponent<CurveComponent>(CurveData);
}

CurveActor::CurveActor(const TArray<FVector>& CurveData)
{
	auto CurveObject = NewObject<Curve>(CurveData);
	CComponent = AddComponent<CurveComponent>(CurveObject);
}

CurveActor::CurveActor(const TArray<FVector>& CurveData, bool bClosed)
{
	auto CurveObject = NewObject<Curve>(CurveData, bClosed);
	CComponent = AddComponent<CurveComponent>(CurveObject);
}

ObjectPtr<CurveComponent> CurveActor::GetCurveComponent()
{
	return CComponent;
}

TArray<FVector> CurveActor::SampleWithEqualChordLength(int Samples) const
{
	auto SamplesData = CComponent->GetCurveData()->SampleWithEqualChordLength(Samples);
	auto Transform = GetFTransform();
	std::ranges::for_each(SamplesData, [&Transform](FVector& Sample) { Sample = Transform * Sample; });
	return SamplesData;
}

FVector CurveActor::Sample(double u) const
{
    return GetTransform() * CComponent->Sample(u);
}

FVector CurveActor::SampleIndex(int Index) const
{
    return GetTransform() * CComponent->SampleIndex(Index);
}